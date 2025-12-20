#ifndef PARSER_H
#define PARSER_H

typedef struct
{
    char** tokens; // Array of token strings
    int pos;                                // Current position in token array
    int count;                              // Total number of tokens
} Parser;

int tokenize(char *input, Parser *parser);
int isMatch(Parser *p, const char *expected);
int consume_token(Parser *p, const char *expected);
int isValidName(Parser *p);
int parse_name(Parser *p);
int parse_input(Parser *p);
int parse_output(Parser *p);
int parse_atomic(Parser *p);
int parse_cmd_group(Parser *p);
int parse_shell_cmd(Parser *p);
int parse_command(char *input,Parser* parser);
void free_tokens(Parser *parser);
#endif
