#include "headers.h"
#include "activities.h"
// ############## LLM Generated Code Begins ##############
// Global variables for tracking the foreground process and shell's PGID
pid_t fg_pgid = -1;
char fg_command[256] = "";
pid_t shell_pgid;

void init_signal_handlers() {
    // A robust shell should IGNORE interactive signals.
    // The terminal will send them to the foreground child process, not the shell.
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

// Handles the EOF (Ctrl-D) condition
void handle_eof() {
    printf("logout\n");
    kill_all_processes();
    exit(0);
}

// Waits for a foreground process group and handles its state changes
void fg_wait(pid_t pgid, const char *cmd_name) {
    int status;
    fg_pgid = pgid; // Set the global foreground pgid
    strncpy(fg_command, cmd_name, sizeof(fg_command) - 1);

    // WUNTRACED is crucial: it allows waitpid to return if the child is stopped.
    pid_t waited_pid = waitpid(pgid, &status, WUNTRACED); // Use pgid directly

    // After the process has either terminated or stopped, reset the foreground tracker.
    fg_pgid = -1;
    fg_command[0] = '\0';

    if (waited_pid == -1) {
        perror("waitpid failed in fg_wait");
        return;
    }

    // If the process was stopped by a signal (like Ctrl-Z)...
    if (WIFSTOPPED(status)) {
        // Add the process to the background job list with STOPPED state
        int job_id = add_process(pgid, cmd_name, STOPPED);
        if (job_id != -1) {
            // Print the required message
            printf("\n[%d] Stopped %s\n", job_id, cmd_name);
        }
    } else if (WIFEXITED(status)) {
        // Process exited, no need to add to activities list
    } else if (WIFSIGNALED(status)) {
        // Process terminated by signal, no need to add to activities list
    }
}
// ############## LLM Generated Code Ends ################