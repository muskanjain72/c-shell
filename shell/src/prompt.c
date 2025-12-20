#include "headers.h"
#include "prompt.h"
#include <limits.h>
// ############## LLM Generated Code Begins ##############
void prompt(char *curr_path, const char *home_path)
{
    char sysName[2048];
    struct passwd *pwd = getpwuid(getuid());
    char *userName = pwd->pw_name;
    
    // char *userName = getlogin();    this can fail sometimes
    gethostname(sysName, sizeof(sysName));

    char finalPath[4096];
    
    // Check if the current path is a sub-directory of the home path
    if (strncmp(curr_path, home_path, strlen(home_path)) == 0)
    {
        // Replace the home path with a tilde '~'
        snprintf(finalPath, sizeof(finalPath), "~%s", curr_path + strlen(home_path));
    }
    else
    {
        strcpy(finalPath, curr_path);
    }
    
    printf("<%s@%s:%s> ", userName, sysName, finalPath);
    return;
}
// ############## LLM Generated Code Ends ################