#ifndef SIGNALS_H
#define SIGNALS_H

#include <sys/types.h>

extern pid_t shell_pgid;
extern pid_t fg_pgid;
extern char fg_command[256];

void init_signal_handlers();
void handle_eof();
void fg_wait(pid_t pgid, const char *cmd_name);

#endif