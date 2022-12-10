#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../headers/constants.h"
#include "../headers/launch.h"
#include "../headers/builtins.h"

char *history_path = NULL;


/* Returns the number of builtin commands available for shell */
int num_builtins()
{
    return sizeof(builtins) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == NULL || strcmp(args[1],"~")==0)
    {
        // If we write no path (only 'cd'), then go to the home directory
        chdir(getenv("HOME"));
        return 1;
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            fprintf(stderr, RED "shell: %s\n" RESET, strerror(errno));
        }
    }

    return 1;
}

/* Function that shows the help message */
int shell_help(char **args)
{
    int i;
    printf(BOLD "\nshell: A minimalistic shell written in C."
                "\n" RESET
                "Builtin commands:\n"
                "cd - Change dicrectory\n"
                "history - Show history from ~/.shell_history\n"
                "q- Exit this shell"
                "help - Show this help\n\n");

    return 1;
}

void shell_exit_on_SIGINT(int signal)
{
    exit(0);
}

int shell_history(char **args)
{
    char *history_args[4] = {"cat", "-n", history_path, NULL};
    return launch(history_args, STDOUT_FILENO, shell_FG);
}
