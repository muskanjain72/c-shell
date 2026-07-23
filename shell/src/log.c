#include "headers.h"

void execute_command_group(char **group_tokens, int is_background);

LogEntry command_log[MAX_LOG_ENTRIES]; //to store the recent entries
int log_count = 0; //number of entries
int log_start = 0; //start index
char log_file_path[1024]; //path of the log file

int initialize_log() {
    char *log_path = get_log_file_path();  //stores the log-file path
    if (!log_path) {
        fprintf(stderr, "Error: Could not determine log file path\n");
        return 0;
    }
    strcpy(log_file_path, log_path);  //stores the log file path
    load_log_from_file();  //loads the log from the file
    return 1;
}

char *get_log_file_path() {
    static char path[LOG_PATH_SIZE];
    snprintf(path, sizeof(path), "%s/.shell_log", home_dir);
    return path;
    //always store in thr home directory of shell with its name as shell_log
}

void add_to_log(const char *command) {
    if (!command || strlen(command) == 0) return;

    if (log_count > 0) {
        //if its the same as last command, dont add it and return
        int last_index = (log_start + log_count - 1) % MAX_LOG_ENTRIES;
        if (strcmp(command_log[last_index].command, command) == 0) return;
    }
    //if the log is full, overwrite the oldest entry
    int index = (log_start + log_count) % MAX_LOG_ENTRIES;
    if (log_count < MAX_LOG_ENTRIES) {
        log_count++;
    } else {
        log_start = (log_start + 1) % MAX_LOG_ENTRIES;
    }
    strncpy(command_log[index].command, command, MAX_CMD_LENGTH - 1);
    command_log[index].command[MAX_CMD_LENGTH - 1] = '\0';

    save_log_to_file();
}

int execute_log(char **tokens) {
    if (tokens[1] == NULL) {
        print_log();
        return 1;
    }
    if (strcmp(tokens[1], "purge") == 0) {
        if (tokens[2] != NULL) {
            printf("log: Invalid Syntax!\n");
            return 0;
        }
        purge_log();
        return 1;
    }
    if (strcmp(tokens[1], "execute") == 0) {
        if (tokens[2] == NULL || tokens[3] != NULL) {
            printf("log: Invalid Syntax!\n");
            return 0;
        }
        char *endptr;
        long index = strtol(tokens[2], &endptr, 10);
        if (*endptr != '\0' || index <= 0 || index > log_count) {
            printf("log: Invalid index!\n");
            return 0;
        }
        char *command_to_execute = get_command_from_log((int)index);
        if (command_to_execute) {
            
            
            char* cmd_copy = strdup(command_to_execute);
            //why we strdup? bcz parse command changes the string by doing modifications in tokens
            Parser parser;
            // Validate the command from the log before executing that is its psbl, then the command isnt valid
            if (parse_command(cmd_copy, &parser)) {
                execute_command_group(parser.tokens, 0);
                free_tokens(&parser);
            } else {
                printf("Invalid command in log: %s\n", command_to_execute);
                free_tokens(&parser);
            }
            free(cmd_copy);
        }
        return 1;
    }
    
    printf("log: Invalid Syntax!\n");
    return 0;
}

void print_log() {
    if (log_count == 0) return;
    for (int i = 0; i < log_count; i++) {
        int index = (log_start + i) % MAX_LOG_ENTRIES;
        printf("%s\n", command_log[index].command);
    }
}

void purge_log() {
    log_count = 0;
    log_start = 0;
    // Overwrite the file with nothing
    FILE *file = fopen(log_file_path, "w");
    if (file) fclose(file);
}

char *get_command_from_log(int index) {
    if (index < 1 || index > log_count) return NULL;
    // Newest is 1, which is at the end of the current log.
    int actual_index = (log_start + log_count - index) % MAX_LOG_ENTRIES;
    return command_log[actual_index].command;
}

void save_log_to_file() {
    //everytime we re-write the whole commands
    FILE *file = fopen(log_file_path, "w");
    if (!file) return;
    for (int i = 0; i < log_count; i++) {
        int index = (log_start + i) % MAX_LOG_ENTRIES;
        fprintf(file, "%s\n", command_log[index].command);
    }
    fclose(file);
}

void load_log_from_file() {
    //from the file u copy it in the array
    FILE *file = fopen(log_file_path, "r");
    if (!file) return;
    char line[MAX_CMD_LENGTH];
    log_count = 0;
    log_start = 0;
    while (fgets(line, sizeof(line), file) && log_count < MAX_LOG_ENTRIES) {
        line[strcspn(line, "\n")] = 0;
        //fgests take an extra endline character so we remove it here 
        if (strlen(line) > 0) {
            strncpy(command_log[log_count].command, line, MAX_CMD_LENGTH - 1);
            command_log[log_count].command[MAX_CMD_LENGTH - 1] = '\0';
            log_count++;
        }
    }
    fclose(file);
}