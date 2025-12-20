#ifndef PING_H
#define PING_H

#include <sys/types.h> // Required for pid_t

void ping_process(pid_t pid, int signal_number);

#endif
