#include "headers.h"
// ############## LLM Generated Code Begins ##############
// Forward declarations
int parse_flags(char **tokens, int *show_hidden, int *line_by_line, int *path_index);
int reveal_directory(const char *path, int show_hidden, int line_by_line);
int compare_strings(const void *a, const void *b);
void printEntries(char **entries, int count, int line_by_line);


int execute_reveal(char** tokens)
{
    int show_hidden = 0;  // -a flag
    int line_by_line = 0; // -l flag
    int path_index = -1;  // Index of path argument
    if (!parse_flags(tokens, &show_hidden, &line_by_line, &path_index))
    {
        printf("reveal: Invalid Syntax!\n");
        return 0;
    }

    char path_buffer[MAX_PATH];
    const char *target_path = NULL;

    if (path_index == -1) {
        target_path = "."; // Default to current directory
    } else {
        const char *arg = tokens[path_index];
        if (strcmp(arg, "~") == 0) {
            target_path = home_dir;
        } else if (strcmp(arg, "-") == 0) {
            if (!has_previous) {
                printf("No such directory!\n");
                return 0;
            }
            target_path = previous_dir;
        } else {
            target_path = arg;
        }
    }

    // Resolve path to handle . and .. correctly before passing to opendir
    if (realpath(target_path, path_buffer) == NULL) {
        printf("No such directory!\n");
        return 0;
    }

    return reveal_directory(path_buffer, show_hidden, line_by_line);
}

int reveal_directory(const char *path, int show_hidden, int line_by_line)
{
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("No such directory!\n");
        return 0;
    }

    char **entries = malloc(2048 * sizeof(char *));
    if (entries == NULL) {
        perror("malloc");
        closedir(dir);
        return 0;
    }
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && count < 2048) {
        // If we are not showing hidden files, skip any file starting with a dot.
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        entries[count] = strdup(entry->d_name);
        if (entries[count] == NULL) {
            perror("strdup");
            // Free previously allocated memory
            for (int i = 0; i < count; i++) free(entries[i]);
            free(entries);
            closedir(dir);
            return 0;
        }
        count++;
    }

    closedir(dir);

    qsort(entries, count, sizeof(char *), compare_strings);

    printEntries(entries, count, line_by_line);

    for (int i = 0; i < count; i++) {
        free(entries[i]);
    }
    free(entries);

    return 1;
}

int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void printEntries(char **entries, int count, int line_by_line) {
    if (line_by_line) {
        for (int i = 0; i < count; i++) {
            printf("%s\n", entries[i]);
        }
    } else {
        for (int i = 0; i < count; i++) {
            printf("%s%s", entries[i], (i == count - 1) ? "" : " ");
        }
        if (count > 0) {
            printf("\n");
        }
    }
}

int parse_flags(char **tokens, int *show_hidden, int *line_by_line, int *path_index) {
    *show_hidden = 0;
    *line_by_line = 0;
    *path_index = -1;
    int path_count = 0;

    for (int i = 1; tokens[i] != NULL; i++) {
        if (tokens[i][0] == '-') {
            if (strlen(tokens[i]) == 1) { // Just a '-', treat as path
                 if (path_count == 0) {
                    *path_index = i;
                    path_count++;
                } else {
                    return 0; // Too many path arguments
                }
                continue;
            }
            for (int j = 1; tokens[i][j] != '\0'; j++) {
                if (tokens[i][j] == 'a') {
                    *show_hidden = 1;
                } else if (tokens[i][j] == 'l') {
                    *line_by_line = 1;
                } else {
                    return 0; // Invalid flag
                }
            }
        } else {
            if (path_count == 0) {
                *path_index = i;
                path_count++;
            } else {
                return 0; // Too many path arguments
            }
        }
    }
    return 1;
}
// ############## LLM Generated Code Ends ################
