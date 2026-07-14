// ############## LLM Generated Code Begins ##############
#include "headers.h"
#include <ctype.h>

int tokenize(char *input, Parser *parser) {
    parser->count = 0;
    parser->pos = 0;
    parser->tokens = malloc(MAX_TOKENS * sizeof(char *));
    if (!parser->tokens) {
        perror("malloc");
        return 0;
    }

    char *current = input;
    while (*current != '\0' && parser->count < MAX_TOKENS - 1) {

        while (*current && isspace(*current)) {
            current++;
        }

        if (*current == '\0') break;

        char *token_start = current;
        if (*current == '"') { // Quoted token
            token_start++; // Skip the opening quote
            current++;
            while (*current && *current != '"') {
                current++;
            }

            char* token = malloc(current - token_start + 1);
            strncpy(token, token_start, current - token_start);
            token[current - token_start] = '\0';
            parser->tokens[parser->count++] = token;
            if (*current == '"') current++; // Skip the closing quote

        } else if (strchr("|&;<>", *current)) { // Special character token
            int len = 1;
            if (*current == '>' && *(current + 1) == '>') {
                len = 2;
            }
            char* token = malloc(len + 1);
            strncpy(token, current, len);
            token[len] = '\0';
            parser->tokens[parser->count++] = token;
            current += len;

        } else { // Normal token
            while (*current && !isspace(*current) && !strchr("|&;<>\"", *current)) {
                current++;
            }
            char* token = malloc(current - token_start + 1);
            strncpy(token, token_start, current - token_start);
            token[current - token_start] = '\0';
            parser->tokens[parser->count++] = token;
        }
    }

    parser->tokens[parser->count] = NULL;
    return parser->count;
}


// Check if current token matches expected string
int isMatch(Parser *p, const char *expected)
{
    if (p->pos >= p->count)
    {
        return 0;
    }
    return strcmp(p->tokens[p->pos], expected) == 0;
}

// Consume current token if it matches expected
int consume_token(Parser *p, const char *expected)
{
    if (isMatch(p, expected))
    {
        p->pos++;
        return 1;
    }
    return 0;
}

// Checks if the current token is a valid name
int isValidName(Parser *p)
{
    if (p->pos >= p->count)
    {
        return 0;
    }
    char *token = p->tokens[p->pos];
    if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 ||
        strcmp(token, "|") == 0 || strcmp(token, "&") == 0 || strcmp(token, ";") == 0) {
        return 0;
    }
    return 1;
}

// Grammar: name -> r"[^|&><;\s]+"
int parse_name(Parser *p)
{
    if (isValidName(p))
    {
        p->pos++;
        return 1;
    }
    return 0;
}

// Grammar: input -> < name
int parse_input(Parser *p)
{
    int saved_pos = p->pos;
    if (consume_token(p, "<"))
    {
        if (parse_name(p))
        {
            return 1;
        }
    }
    p->pos = saved_pos;
    return 0;
}

// Grammar: output -> > name | >> name
int parse_output(Parser *p)
{
    int saved_pos = p->pos;
    if (consume_token(p, ">") || consume_token(p, ">>"))
    {
        if (parse_name(p))
        {
            return 1;
        }
    }
    p->pos = saved_pos;
    return 0;
}

// Grammar: atomic -> name (name | input | output)*
int parse_atomic(Parser *p)
{
    if (!parse_name(p)) return 0;

    while (p->pos < p->count) {
        int current_pos = p->pos;
        if (parse_input(p) || parse_output(p) || parse_name(p)) {
            continue;
        }
        p->pos = current_pos;
        break;
    }
    return 1;
}

// Grammar: cmd_group -> atomic (\| atomic)*
int parse_cmd_group(Parser *p)
{
    if (!parse_atomic(p)) return 0;

    while (p->pos < p->count && isMatch(p, "|")) {
        consume_token(p, "|");
        if (!parse_atomic(p)) return 0;
    }
    return 1;
}

// Grammar: shell_cmd -> cmd_group (('&' | ';') cmd_group)* '&'?
int parse_shell_cmd(Parser *p)
{
    if (!parse_cmd_group(p)) return 0;

    while (p->pos < p->count) {
        if (isMatch(p, ";")) {
            p->pos++; // consume ';'
            if (p->pos == p->count) break; // trailing ';' is allowed
            if (!parse_cmd_group(p)) return 0;
        } else if (isMatch(p, "&")) {
            p->pos++; // consume '&'
            if (p->pos == p->count) break; // trailing '&' (the '&?' at the end)
            if (!parse_cmd_group(p)) return 0;
        } else {
            break; // leftover tokens — will fail the pos != count check
        }
    }
    return 1;
}

// This is the main validation function.
int parse_command(char *input, Parser *parser)
{
    if (tokenize(input, parser) == 0) {
        free_tokens(parser);
        return 1; // Empty input is valid and successful.
    }

    parser->pos = 0;
    if (!parse_shell_cmd(parser)) {
        return 0;
    }

    if (parser->pos != parser->count) {
        return 0;
    }

    parser->pos = 0; // Reset for the execution phase.
    return 1;
}

void free_tokens(Parser *parser)
{
    if (parser && parser->tokens)
    {
        for (int i = 0; i < parser->count; i++)
        {
            if (parser->tokens[i] != NULL)
            {
                free(parser->tokens[i]);
            }
        }
        free(parser->tokens);
        parser->tokens = NULL;
    }
}
// ############## LLM Generated Code Ends ################