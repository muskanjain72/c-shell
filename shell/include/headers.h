#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_ARGS 128
#define MAX_COMMANDS 50
#define MAX_LOG_ENTRIES 15
#define MAX_TOKENS 128
#define MAX_TOKEN_LEN 256

// Include other headers
#include "activities.h"
// #include "background.h"
// #include "execution.h"
#include "hop.h"
#include "input.h"
#include "jobs.h"
#include "log.h"
#include "parser.h"
#include "ping.h"
#include "pipes.h"
#include "prompt.h"
#include "reveal.h"
// #include "sequential.h"
#include "signals.h"

#endif