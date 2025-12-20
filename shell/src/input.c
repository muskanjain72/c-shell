// ############## LLM Generated Code Begins ##############
#include "headers.h"

char *takeInputFromUser() {
    // When the user presses the enter/return key, the shell should consume the input.
    char *text = NULL;
    size_t len = 0;
    ssize_t a = getline(&text, &len, stdin);

    if (a == -1) {
        free(text);
        return NULL;
    }

    // Remove the trailing newline character since it bydefault includes that
    if (a > 0 && text[a - 1] == '\n') {
        text[a - 1] = '\0';
    }
    return text;
}
// ############## LLM Generated Code Ends ################