

// ############## LLM Generated Code Begins ##############
#include "headers.h"
#include "activities.h"
#include "signal.h" // For fg_wait and shell_pgid
#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void execute_fg(char **tokens);
void execute_bg(char **tokens);


void execute_jobs_command(char **tokens) {
    if (tokens[0] == NULL) return;

    if (strcmp(tokens[0], "fg") == 0) {
        execute_fg(tokens);
    } else if (strcmp(tokens[0], "bg") == 0) {
        execute_bg(tokens);
    }
}


void execute_fg(char **tokens) {
    if (tokens[1] != NULL && tokens[2] != NULL) {
        printf("fg: too many arguments\n");
        return;
    }

    Process *job = NULL;
    if (tokens[1] == NULL) {
        job = get_most_recent_job();
    } else {
        char *endptr;
        long job_num = strtol(tokens[1], &endptr, 10);
        if (*endptr != '\0') {
            printf("fg: invalid job number\n");
            return;
        }
        job = get_job_by_number((int)job_num);
    }

    if (job == NULL) {
        printf("No such job\n");
        return;
    }

    pid_t pgid = job->pid;
    char cmd_name[256];
    strcpy(cmd_name, job->command);
    ProcessState state = job->state;

    printf("%s\n", cmd_name);

    // Remove from background list as it is now in the foreground
    remove_process(pgid);
    
    tcsetpgrp(STDIN_FILENO, pgid);

    if (state == STOPPED) {
        if (kill(-pgid, SIGCONT) < 0) {
            perror("fg: kill (SIGCONT)");
            tcsetpgrp(STDIN_FILENO, shell_pgid);
            return;
        }
    }
    
    fg_wait(pgid, cmd_name);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
}


void execute_bg(char **tokens) {
    if (tokens[1] != NULL && tokens[2] != NULL) {
        printf("bg: too many arguments\n");
        return;
    }

    Process *job = NULL;
    if (tokens[1] == NULL) {
        job = get_most_recent_job();
    } else {
        char *endptr;
        long job_num = strtol(tokens[1], &endptr, 10);
        if (*endptr != '\0') {
            printf("bg: invalid job number\n");
            return;
        }
        job = get_job_by_number((int)job_num);
    }

    if (job == NULL) {
        printf("No such job\n");
        return;
    }

    if (job->state == RUNNING) {
        printf("Job already running\n");
        return;
    }

    if (kill(-job->pid, SIGCONT) < 0) {
        perror("bg: kill (SIGCONT)");
        return;
    }

    job->state = RUNNING;
    printf("[%d] %s &\n", job->job_number, job->command);
}

// ############## LLM Generated Code Ends ################