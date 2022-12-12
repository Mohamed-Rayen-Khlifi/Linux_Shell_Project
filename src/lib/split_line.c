#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include "../headers/constants.h"
/* Function that splits the line based on delimeters defined in constants.h */
// Splits commands into tokens and groups these tokens in a token table then returns it
char **split_line(char *line)
{
    char **tokens = malloc(sizeof(char *) * TOKEN_BUFSIZE);
    char *token;

    int bufsize_copy = TOKEN_BUFSIZE;
    int pos = 0;

    if (!tokens)
    {
        fprintf(stderr, RED "Memory allocation failed." RESET);
        exit(EXIT_FAILURE);
    }
    
    // Split inpute line in tokens depending on delimiters
    token = strtok(line, TOKEN_DELIMS);
    while (token != NULL)
    {
        tokens[pos] = token;
        pos++;

        // Reallocing memory space for the tokens table
        if (pos >= bufsize_copy)
        {
            bufsize_copy = bufsize_copy * 2;
            tokens = realloc(tokens, sizeof(char *) * bufsize_copy);

            if (!tokens)
            {
                fprintf(stderr, RED "Memory allocation failed." RESET);
                exit(EXIT_FAILURE);
            }
        }

        // Get the next token
        token = strtok(NULL, TOKEN_DELIMS);
    }

    tokens[pos] = NULL;
    return tokens;
}