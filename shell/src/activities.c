
// ############## LLM Generated Code Begins ##############

#include "headers.h"
#include "activities.h"
#include <string.h> // For strcmp
#include <stdlib.h> // For qsort, malloc, free

#ifndef WCONTINUED
#define WCONTINUED 8 // Define if not present on your system
#endif

Process *head = NULL;

static int job_counter = 1;

static int cmp_process(const void *a, const void *b) {
    Process *pa = *(Process **)a;
    Process *pb = *(Process **)b;
    return strcmp(pa->command, pb->command);
}

void remove_process(pid_t pid) {
    Process *curr = head, *prev = NULL;
    while (curr) {
        if (curr->pid == pid) {
            if (prev) prev->next = curr->next;
            else head = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

int add_process(pid_t pid, const char *cmd, ProcessState state) {
    Process *new_proc = malloc(sizeof(Process));
    if (!new_proc) {
        perror("malloc failed");
        return -1;
    }
    new_proc->pid = pid;
    new_proc->job_number = job_counter;
    strncpy(new_proc->command, cmd, 255);
    new_proc->command[255] = '\0';
    new_proc->state = state;
    new_proc->next = head;
    head = new_proc;
    return job_counter++;
}

void update_processes() {
    Process *curr = head;
    int status;
    while (curr != NULL) {
        Process* next_proc = curr->next;
        pid_t result = waitpid(curr->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == curr->pid) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                remove_process(curr->pid);
            } else if (WIFSTOPPED(status)) {
                curr->state = STOPPED;
            } else if (WIFCONTINUED(status)) {
                curr->state = RUNNING;
            }
        }
        curr = next_proc;
    }
}

Process* get_job_by_number(int job_number) {
    Process *current = head;
    while (current != NULL) {
        if (current->job_number == job_number) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_activities() {
    update_processes();
    int count = 0;
    Process *curr = head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    if (count == 0) return;

    Process **arr = malloc(count * sizeof(Process *));
    if (!arr) { perror("malloc"); return; }

    curr = head;
    for (int i = 0; i < count; i++) {
        arr[i] = curr;
        curr = curr->next;
    }
    qsort(arr, count, sizeof(Process *), cmp_process);
    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", arr[i]->pid, arr[i]->command, arr[i]->state == RUNNING ? "Running" : "Stopped");
    }
    free(arr);
}

void kill_all_processes() {
    Process *curr = head;
    while (curr) {
        kill(curr->pid, SIGKILL);
        curr = curr->next;
    }
}


Process* get_most_recent_job(void) {
    if (!head) {
        return NULL;
    }
    Process *curr = head;
    Process *most_recent = head;

    while (curr) {
        if (curr->job_number > most_recent->job_number) {
            most_recent = curr;
        }
        curr = curr->next;
    }
    return most_recent;
}
// ############## LLM Generated Code Ends ################