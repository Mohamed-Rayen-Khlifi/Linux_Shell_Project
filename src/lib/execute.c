#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../headers/pipe_launch.h"
#include "../headers/constants.h"
#include "../headers/launch.h"
#include "../headers/builtins.h"


/* Function pointers that correspond to the buitins */
int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_history,
};


/* list of builtins in the shell */
char *builtins[] = {
    "cd",
    "help",
    "history",
    "exit"};

/* Returns the number of builtin commands available for shell */
int num_builtins()
{
    return sizeof(builtins) / sizeof(char *);
}

/* Function responsible for parsing the arguments */
int execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        return 1;
    }

    i = 0;
    while (args[i])
    {
        i++;
    }

    /* launch process in background */
    if (!strcmp("&", args[--i]))
    {
        args[i] = NULL;
        return launch(args, STDOUT_FILENO, shell_BG);
    }

    for (i = 0; i < num_builtins(); i++)
    {
        // strcmp("a","a") == 0
        if (!strcmp(args[0], builtins[i]))
        {
            return (*builtin_func[i])(args);
        }
    }

    int j = 0;

    while (args[j] != NULL)
    {
        // for `>` operator for redirection (stdout)
        if (!strcmp(">", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG);
        }
        // for `>>` operator for redirection (stdout with append)
        else if (!strcmp(">>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "a+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDOUT);
        }
        // for `2>` operator for redirection (stderr)
        else if (!strcmp("2>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR);
        }
        // for `>&` operator for redirection (stdout and stderr)
        else if (!strcmp(">&", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR | shell_STDOUT);
        }
        // for `<` operator for redirection (stdin)
        else if (!strcmp("<", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "r"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDIN);
        }
        // for piping
        else if (!strcmp("|", args[j]))
        {
            char **arg2;
            int i = 0;
            args[j] = NULL;
            arg2 = &args[j + 1];

            return pipe_launch(args, arg2);
        }
        j++;
    }

    return launch(args, STDOUT_FILENO, shell_FG);
}
