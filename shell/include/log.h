#ifndef LOG_H
#define LOG_H
#define MAX_LOG_ENTRIES 15
#define MAX_CMD_LENGTH 1024
#define EXECUTE_LOG_CMD 2
#define LOG_PATH_SIZE (MAX_PATH + 16) // Increased size to prevent buffer overflow
typedef struct
{
    char command[MAX_CMD_LENGTH];
} LogEntry;

extern LogEntry command_log[MAX_LOG_ENTRIES];
extern int log_count;
extern int log_start;
extern char log_file_path[1024];

int initialize_log();
char *get_log_file_path();
void add_to_log(const char *command);
int execute_log(char **tokens);
void print_log();
void purge_log();
bool shouldLogCommand(char *input);


char *get_command_from_log(int index);
void save_log_to_file();
void load_log_from_file();

#endif
