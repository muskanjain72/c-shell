
#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <sys/types.h>

// Enum for process state
typedef enum {
    RUNNING,
    STOPPED
} ProcessState;

// Struct for background processes
typedef struct Process {
    pid_t pid;
    char command[256];
    ProcessState state;
    int job_number;
    struct Process *next;
} Process;


extern Process *head;


int add_process(pid_t pid, const char *cmd, ProcessState state);
void update_processes();
void kill_all_processes();
void remove_process(pid_t pid);
Process* get_job_by_number(int job_number);
void list_activities(); 
Process* get_most_recent_job(void); 

#endif
