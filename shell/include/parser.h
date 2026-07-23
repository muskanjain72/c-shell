#ifndef PARSER_H
#define PARSER_H

typedef struct
{
    char** tokens;          //array of token strings
    int pos;                //current position in token array
    int count;              //total number of tokens
} Parser;

int tokenize(char *input, Parser *parser);  //tokenizes the input string
int isMatch(Parser *p, const char *expected); //checks if the current token matches the expected string
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
