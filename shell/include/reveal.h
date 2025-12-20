#ifndef REVEAL_H
#define REVEAL_H
int execute_reveal(char **tokens);
int parse_flags(char **tokens, int *show_hidden, int *line_by_line, int *path_index);
int reveal_directory(const char *path, int show_hidden, int line_by_line);
int compare_strings(const void *a, const void *b);
void printEntries(char **entries, int count, int line_by_line);

#endif