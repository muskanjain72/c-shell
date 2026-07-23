

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
        //strtol because it detects invalid inputs as well which atoi cant
        if (*endptr != '\0') {
            printf("fg: invalid job number\n");
            return;
        }
        job = get_job_by_number((int)job_num);
    }

    if (job == NULL) {
        //job not found
        printf("No such job\n");
        return;
    }

    pid_t pgid = job->pid; // process group id
    char cmd_name[256]; // command name
    strcpy(cmd_name, job->command); //copy command name
    ProcessState state = job->state; //stores the current state of process

    printf("%s\n", cmd_name);

    // Remove from background list as it is now in the foreground
    remove_process(pgid);
    
    //gives control of terminal to the foreground job -> now input, interupt is for it
    tcsetpgrp(STDIN_FILENO, pgid);

    if (state == STOPPED) {
        if (kill(-pgid, SIGCONT) < 0) {
            // Negative PID means send the signal to the entire process group, not just one process.
            perror("fg: kill (SIGCONT)");
            tcsetpgrp(STDIN_FILENO, shell_pgid);
            return;
        }
    }
    
    //waits for the process to finish
    fg_wait(pgid, cmd_name);
    
    //back to shell
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
    //set the background job to running state
    printf("[%d] %s &\n", job->job_number, job->command);
}
