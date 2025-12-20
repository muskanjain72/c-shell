// ############## LLM Generated Code Begins ##############
#include "headers.h"

extern char home_dir[MAX_PATH];
extern char current_dir[MAX_PATH];
extern char previous_dir[MAX_PATH];
extern int has_previous;

void hopToHome();
void hopToParent();
void hopToPrevious();
void hopToPath(char *path);
void print_error(const char *dir);

int execute_hop(char **tokens) {
    char old_cwd[MAX_PATH];
    if (getcwd(old_cwd, sizeof(old_cwd)) == NULL) {
        perror("getcwd failed");
        return 0;
    }

    int chdir_success = 0; // Flag to indicate if any chdir was successful

    if (tokens[1] == NULL) {
        if (chdir(home_dir) == 0) chdir_success = 1;
        else print_error(home_dir);
    } else {
        for (int i = 1; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "~") == 0) {
                if (chdir(home_dir) == 0) chdir_success = 1;
                else print_error(home_dir);
            } else if (strcmp(tokens[i], ".") == 0) {
                // Do nothing, already in current directory
            } else if (strcmp(tokens[i], "..") == 0) {
                if (chdir("..") == 0) chdir_success = 1;
                else print_error("..");
            } else if (strcmp(tokens[i], "-") == 0) {
                if (!has_previous) {
                    printf("No previous directory!\n");
                } else {
                    if (chdir(previous_dir) == 0) chdir_success = 1;
                    else print_error(previous_dir);
                }
            } else {
                if (chdir(tokens[i]) == 0) chdir_success = 1;
                else print_error(tokens[i]);
            }
        }
    }

    // If any chdir was successful and the current directory is actually different from old_cwd
    char new_cwd[MAX_PATH];
    if (getcwd(new_cwd, sizeof(new_cwd)) == NULL) {
        perror("getcwd update failed");
        return 0;
    }

    if (chdir_success && strcmp(old_cwd, new_cwd) != 0) {
        strcpy(previous_dir, old_cwd);
        has_previous = 1;
    }

    strcpy(current_dir, new_cwd); // Always update current_dir to reflect actual CWD

    return 1;
}

void hopToHome() {
    if (chdir(home_dir) != 0) {
        print_error(home_dir);
    }
}

void hopToParent() {
    if (chdir("..") != 0) {
        print_error("..");
    }
}

void hopToPrevious() {
    if (!has_previous) {
        printf("No previous directory!\n");
        return;
    }
    if (chdir(previous_dir) != 0) {
        print_error(previous_dir);
    }
}

void hopToPath(char *path) {
    if (chdir(path) != 0) {
        print_error(path);
    }
}

void print_error(const char *dir) {
    if (errno == ENOENT) {
        printf("No such directory!\n");
    } else {
        // Other errors like permission denied
        perror("hop");
    }
}
// ############## LLM Generated Code Ends ################