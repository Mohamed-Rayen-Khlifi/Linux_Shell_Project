#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
//#include <stdlib.h>
#include "../headers/pipe_launch.h"
#include "../headers/constants.h"
#include "../headers/launch.h"
#include "../headers/builtins.h"

// Component responsible for treating special cases and launching processes using the launch component


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

    // Checking builtin commands

    for (i = 0; i < num_builtins(); i++)
    {
        // strcmp("a","a") == 0
        if (!strcmp(args[0], builtins[i]))
        {
            return (*builtin_func[i])(args);
        }
    }

    int j = 0;
    // Redirection
    while (args[j] != NULL)
    {
        // for `>` operator for redirection (stdout)
        if (!strcmp(">", args[j]))
        {
            //File descriptor used for redirection
            int fd = open(args[j+1],O_WRONLY | O_CREAT | O_TRUNC, 0777);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDOUT);

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





