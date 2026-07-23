#include "headers.h"
#include "prompt.h"
#include "input.h"
#include "log.h"

char home_dir[MAX_PATH];  //stores the hoem directory
char current_dir[MAX_PATH]; //everytime we change our directory, it gets updated
char previous_dir[MAX_PATH]; //if any previous directory has been there
int has_previous = 0; //if there is any previous directory

void execute_command_group(char **group_tokens, int is_background);

void initialize_all_state()
{
    if (getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        exit(1); //save the cwd as home directory
    }
    strcpy(current_dir, home_dir);
    initialize_log(); //loads the log file into the array for O(1) modification and access
}

int main(void)
{
    init_signal_handlers(); //initialises signal handlers

    
    shell_pgid = getpid();  //stores the parent process id
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0)
    {
        perror("tcsetpgrp");
        //tcsetpgrp() gives terminal control to the shell's process group, making it the foreground process group so it can interact with the user.
        // not fatal: continue even if it fails
    }

    initialize_all_state();

    while (1)
    {
        //after every command, update_process() will check if any background jobs are completed
        update_processes(); // Check for completed background jobs
        prompt(current_dir, home_dir); //prints the prompt

        char *input = takeInputFromUser(); //take input from user
        if (input == NULL)
        { // EOF (Ctrl+D)
            handle_eof(); //handles the EOF condition
        }

        if (strlen(input) == 0)
        {
            free(input);
            continue;
        }

        // Check if the command is a 'log' command before adding it to history.
        //makes an independant copy of input
        char *temp_input_for_log_check = strdup(input);
        char *first_word = strtok(temp_input_for_log_check, " 	\n");
        //split by endline or space or tab(4 spaces)
        if (first_word == NULL || strcmp(first_word, "log") != 0)
        {
            add_to_log(input);
        }
        free(temp_input_for_log_check);

        Parser parser;
        // First, validate the entire command line for correct syntax
        if (!parse_command(input, &parser))
        {
            printf("Invalid Syntax!\n");
            free_tokens(&parser);
            free(input);
            continue; // Skip execution and get new input
        }

        int start = 0;
        for (int i = 0; i <= parser.count; i++)
        {
            int is_background = 0;
            int is_separator = 0;

            if (i == parser.count || strcmp(parser.tokens[i], ";") == 0 || strcmp(parser.tokens[i], "&") == 0)
            {
                is_separator = 1;
                if (i < parser.count && strcmp(parser.tokens[i], "&") == 0)
                {
                    is_background = 1;
                }
            }

            if (is_separator)
            {
                if (i > start)
                {
                    parser.tokens[i] = NULL; // Terminate the current command group
                    char **cmd_group = &parser.tokens[start];
                    execute_command_group(cmd_group, is_background);
                }
                start = i + 1;
            }
        }

        free_tokens(&parser);
        free(input);
    }

    return 0;
}

void execute_command_group(char **group_tokens, int is_background)
{
    if (group_tokens[0] == NULL)
        return;

    // The 'log' command is special and should be handled here before the fork.
    if (strcmp(group_tokens[0], "log") == 0)
    {
        execute_log(group_tokens);
        return;
    }

    if (strcmp(group_tokens[0], "ping") == 0)
    {
        if (group_tokens[1] == NULL || group_tokens[2] == NULL || group_tokens[3] != NULL)
        {
            printf("Invalid Syntax!\n");
            return;
        }
        char *endptr1, *endptr2;
        long pid_val = strtol(group_tokens[1], &endptr1, 10);  //string to long
        long sig_val = strtol(group_tokens[2], &endptr2, 10);  //string to long

        if (*endptr1 != '\0' || *endptr2 != '\0')
        {
            printf("Invalid Syntax!\n");
            return;
        }
        ping_process((pid_t)pid_val, (int)sig_val);
        return;
    }

    if (strcmp(group_tokens[0], "fg") == 0 || strcmp(group_tokens[0], "bg") == 0)
    {
        execute_jobs_command(group_tokens);
        return;
    }

    if (strcmp(group_tokens[0], "activities") == 0)
    {
        list_activities();
        return;
    }

    if (strcmp(group_tokens[0], "reveal") == 0)
    {
        execute_reveal(group_tokens);
        return;
    }

    int has_pipe = 0;
    for (int i = 0; group_tokens[i] != NULL; i++)
    {
        if (strcmp(group_tokens[i], "|") == 0)
        {
            has_pipe = 1;
            break;
        }
    }

    if (!has_pipe && !is_background)
    {
        if (strcmp(group_tokens[0], "hop") == 0)
        {
            execute_hop(group_tokens);
            return;
        }
        if (strcmp(group_tokens[0], "exit") == 0 || strcmp(group_tokens[0], "quit") == 0)
        {
            exit(0);
        }
    }

    execute_pipeline(group_tokens, is_background);
}