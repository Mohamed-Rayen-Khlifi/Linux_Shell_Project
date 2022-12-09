#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"
#include "node.h"
//#include "backend.h"

void welcomeScreen()
{
    printf("\n\t===========================================================\n");
    printf("\t          Advanced Operating Systems Project: Basic Shell \n");
    printf("\t              Made by Malak, Maram and Rayen\n");
    printf("\t============================================================\n");
    printf("\n\n");
}

char *read_cmd(void)
{
    char buf[1024];
    char *ptr = NULL;
    char ptrlen = 0;
    // Read input from stdin in 1024-byte chunks and store the input in a buffer
    while (fgets(buf, 1024, stdin))
    {
        int buflen = strlen(buf);

        // Allocating memory space for the first chunk
        if (!ptr)
        {
            ptr = malloc(buflen + 1);
        }

        // Reallocating(Extendingi the buffer) memory space if there are subsequant chunk
        else
        {
            char *ptr2 = realloc(ptr, ptrlen + buflen + 1);

            if (ptr2)
            {
                ptr = ptr2;
            }
            else
            {
                free(ptr);
                ptr = NULL;
            }
        }

        // Failed to allocate memory error handling
        if (!ptr)
        {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        // Copying the chunk read from the input to the buffer
        strcpy(ptr + ptrlen, buf);

        // Checking if the input we’ve got in the buffer ends with "\n"
        if (buf[buflen - 1] == '\n')
        {
            // Cheking if the "\n" is escaped by a backslash character "\\".
            if (buflen == 1 || buf[buflen - 2] != '\\') // The last "\n" is not escaped
            {
                // The input line is complete and we return it to the main() function
                return ptr;
            }

            // The last "\n" is escaped = We typed a back slash and we pressed ENTER
            ptr[ptrlen + buflen - 2] = '\0'; // Remove the two characters "\\" and "\n"
            buflen -= 2;
            print_prompt2(); // Print out PS2 and continue reading input
        }

        // Extending the buffer length
        ptrlen += buflen;
    }

    // The input line is complete and it's returned to the main() function
    return ptr;
}

int parse_and_execute(struct source_s *src)
{
    skip_white_spaces(src);

    struct token_s *tok = tokenize(src);

    if (tok == &eof_token)
    {
        return 0;
    }

    while (tok && tok != &eof_token)
    {
        struct node_s *cmd = parse_simple_command(tok);

        if (!cmd)
        {
            break;
        }

        do_simple_command(cmd);
        free_node_tree(cmd);
        tok = tokenize(src);
    }

    return 1;
}

int main(int argc, char **argv)
{
    char *cmd;
    // REPL Loop
    welcomeScreen();

    do
    {
        print_prompt1();

        // Read a command
        cmd = read_cmd();

        // Error reading the command
        if (!cmd)
        {
            exit(EXIT_SUCCESS);
        }

        // User pressed enter and did nothing "Empty command"
        if (cmd[0] == '\0' || strcmp(cmd, "\n") == 0)
        {
            free(cmd);
            continue;
        }

        // User types quit to leave the REPL
        if (strcmp(cmd, "q\n") == 0)
        {
            free(cmd);
            break;
        }

        // Echo back the command
        // printf("%s\n", cmd);
        struct source_s src;
        src.buffer = cmd;
        src.bufsize = strlen(cmd);
        src.curpos = INIT_SRC_POS;
        parse_and_execute(&src);

        // Free the memory used to store the command
        free(cmd);

    } while (1);

    exit(EXIT_SUCCESS);
}


