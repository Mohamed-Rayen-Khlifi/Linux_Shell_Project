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

void main_loop(void)
{
    char *line, *prompt, *homedir = getenv("HOME");
    char **args;
    int status, index;
    size_t history_path_len = sizeof(char) * PATH_MAX;
    history_path = malloc(history_path_len);

    snprintf(history_path, history_path_len, "%s/%s", homedir, HISTFILE);

    read_history(history_path);

    do
    {
        if ((prompt = get_prompt()) == NULL)
        {
            status = 0;
            fprintf(stderr, RED "shell: Failed to get prompt.\n" RESET);
        }

        line = readline(prompt);
        add_history(line);

        if (!line)
        {
            status = 0;
        }

        else
        {
            write_history(history_path);

            if (strcmp(line, "quit") == 0)
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