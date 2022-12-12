#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../headers/execute.h"
#include "../headers/split_line.h"
#include "../headers/get_prompt.h"
#include "../headers/constants.h"
#include "../headers/builtins.h"
/* The main loop of the shell */
void main_loop(void)
{
    // Signal to close the shell
    signal(SIGINT, shell_exit_on_SIGINT);
    char *line;
    char **args;

    int status, index;

    // get prompt
    char *prompt;

    char *homedir = getenv("HOME");

    size_t history_path_len = sizeof(char) * PATH_MAX;
    history_path = malloc(history_path_len);
    // Formats and stores a series of characters and values in the array buffer.
    snprintf(history_path, history_path_len, "%s/%s", homedir, HISTFILE);

    // snprintf(buffer, max_size_of_buffer,"%s", char_variable_name)

    // read from history file
    read_history(history_path);

    do
    {
        // use GNU's readline() function
        if ((prompt = get_prompt()) == NULL)
        {
            status = 0;
            fprintf(stderr, RED "shell: Failed to get prompt.\n" RESET);
        }
        // Reads user input until the ENTER Key is pressed
        line = readline(prompt);

        // add to history for future use
        add_history(line);

        // EOF
        if (!line)
        {
            status = 0;
        }

        else
        {
            // write to history file from the add_history function
            write_history(history_path);

            // Quiting the shell
            if (line[0] == 'q' && line[1] == 'u' && line[2] == 'i' && line[3] == 't')
            {
                exit(0);
                free(prompt);
            }
       
            else
            {
                args = split_line(line);
            }

            if (args)
            {
                status = execute(args);
                free(args);
            }
            else
            {
                status = 1;
            }
        }

        free(prompt);
        free(line);

    } while (status);

    free(history_path);
}