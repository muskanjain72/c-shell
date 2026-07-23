
#include "headers.h"
#include "execution.h"
#include "activities.h"

char** handle_redirections(char **cmd_tokens, int *in_fd, int *out_fd, int is_background) {
    static char* new_argv[MAX_ARGS];
    int argc = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;
    int input_redirect_found = 0;

    for (int i = 0; cmd_tokens[i] != NULL; i++) {
        if (strcmp(cmd_tokens[i], "<") == 0) {
            input_file = cmd_tokens[++i];
            if (input_file == NULL) {
                printf("No such file or directory\n");
                return NULL;
            }
            input_redirect_found = 1;
        } else if (strcmp(cmd_tokens[i], ">") == 0) {
            output_file = cmd_tokens[++i];
            if (output_file == NULL) {
                printf("Invalid Syntax: No file for output redirection.\n");
                return NULL;
            }
            append_mode = 0;
        } else if (strcmp(cmd_tokens[i], ">>") == 0) {
            output_file = cmd_tokens[++i];
            if (output_file == NULL) {
                printf("Invalid Syntax: No file for append redirection.\n");
                return NULL;
            }
            append_mode = 1;
        } else {
            new_argv[argc++] = cmd_tokens[i];
        }
    }
    new_argv[argc] = NULL;

    if (is_background && !input_redirect_found) {
        *in_fd = open("/dev/null", O_RDONLY);
        if (*in_fd < 0) {
            perror("open /dev/null");
            exit(1);
        }
    } else if (input_file != NULL) {
        *in_fd = open(input_file, O_RDONLY);
        if (*in_fd < 0) {
            printf("No such file or directory\n");
            exit(1);
        }
    }

    if (output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= (append_mode) ? O_APPEND : O_TRUNC;
        *out_fd = open(output_file, flags, 0644);
        if (*out_fd < 0) {
            printf("Unable to create file for writing\n");
            exit(1);
        }
    }

    return new_argv;
}

void execute_pipeline(char **tokens, int is_background) {
    int num_cmds = 0;
    int cmd_starts[MAX_COMMANDS] = {0};
    num_cmds = 1;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;
            if (tokens[i+1] != NULL && strlen(tokens[i+1]) > 0) {
                cmd_starts[num_cmds++] = i + 1;
            } else {
                printf("Invalid Syntax!\n");
                return;
            }
        }
    }

    int prev_pipe_read_end = STDIN_FILENO;
    pid_t pids[num_cmds];
    pid_t pgid = 0;

    for (int i = 0; i < num_cmds; i++) {
        int pipe_fds[2];
        if (i < num_cmds - 1) {
            if (pipe(pipe_fds) < 0) { perror("pipe"); return; }
        }

        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); return; }

        if (pids[i] == 0) { // Child Process
            if (i == 0) {
                pgid = getpid();
                setpgid(0, pgid);
            } else {
                setpgid(0, pgid);
            }

            if (prev_pipe_read_end != STDIN_FILENO) {
                dup2(prev_pipe_read_end, STDIN_FILENO);
                close(prev_pipe_read_end);
            }
            if (i < num_cmds - 1) {
                close(pipe_fds[0]);
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);
            }

            int in_fd = STDIN_FILENO, out_fd = STDOUT_FILENO;
            char **cmd_args = handle_redirections(&tokens[cmd_starts[i]], &in_fd, &out_fd, is_background);

            if (cmd_args == NULL || cmd_args[0] == NULL) exit(1);

            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (out_fd != STDOUT_FILENO) {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            execvp(cmd_args[0], cmd_args);
            printf("Command not found!\n");
            exit(127);
        } else { // Parent Process
            if (i == 0) pgid = pids[i];
            setpgid(pids[i], pgid);

            if (prev_pipe_read_end != STDIN_FILENO) close(prev_pipe_read_end);
            if (i < num_cmds - 1) {
                close(pipe_fds[1]);
                prev_pipe_read_end = pipe_fds[0];
            }
        }
    }

    if (!is_background) {
        // For foreground jobs, wait for the entire pipeline to complete or stop.
        tcsetpgrp(STDIN_FILENO, pgid);
        // forward all signals to this group
        fg_wait(pgid, tokens[cmd_starts[0]]);
        tcsetpgrp(STDIN_FILENO, shell_pgid); //restore it back to shell
    } else {
        // For background, register the job and print its info
        int job_id = add_process(pgid, tokens[cmd_starts[0]], RUNNING);
        if (job_id != -1) {
            printf("[%d] %d\n", job_id, pgid);
        }
    }
}