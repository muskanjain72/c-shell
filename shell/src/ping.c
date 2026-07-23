#include "headers.h"
#include "ping.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h> // For kill()

void ping_process(pid_t pid, int signal_number)
{
    int actual_signal = signal_number % 32;
    //kill system call
    //signal 0 is special ->it doesnt kill, stop or do anything to the process
    //it just asks the kernel, whether this process exists and do i have the permission to send a signal to this process
    if (kill(pid, 0) == -1 && errno == ESRCH)
    {
        //no such process found
        printf("ping: No such process found!\n");
        return;
    }

    if (kill(pid, actual_signal) == -1)
    {
        //other errors like permission denied
        perror("ping");
    }
    else
    {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    }
}