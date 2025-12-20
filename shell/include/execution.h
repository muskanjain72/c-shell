#ifndef EXECUTION_H
#define EXECUTION_H

#include <sys/types.h>

int execute_command(char **tokens, int in_fd, int out_fd);

// Built-in command handlers
int execute_hop(char **tokens);
int execute_reveal(char **tokens);
int execute_log(char **tokens);
void list_activities();
void ping_process(pid_t pid, int sig);
void kill_all_processes();

void fg_wait(pid_t pid, const char* cmd);
void redirect_stdin_to_null();

#endif