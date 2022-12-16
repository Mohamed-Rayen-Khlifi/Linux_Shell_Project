#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
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
    int fd0,fd1,in=0,out=0;
    char input[64],output[64];
    // Redirection
    while (args[j] != NULL)
    {
        // for `>` operator for redirection (stdout)
        if (!strcmp(">", args[j]))
        {
            // File descriptor used for redirection
            /*int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG); */ 
            args[j]=NULL;
            strcpy(output,args[j+1]);
            out=2;
        }
        // for `>>` operator for redirection (stdout with append)
       /* else if (!strcmp(">>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "a+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDOUT);
        }*/
        // for `2>` operator for redirection (stderr)
        
        /* else if (!strcmp("2>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR);
        }
        */
        // for `>&` operator for redirection (stdout and stderr)
        
        /* else if (!strcmp(">&", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR | shell_STDOUT);
        }
        */
        // for `<` operator for redirection (stdin)
        else if (!strcmp("<", args[j]))
        {
            /* int fd = fileno(fopen(args[j + 1], "r"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDIN); */

            args[j]=NULL;
            strcpy(input,args[j+1]);
            in=2;  

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

    //if '<' char was found in string inputted by user
    if(in)
    {   

        // fdo is file-descriptor
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
            perror("Couldn't open input file");
            exit(0);
        }           
        // dup2() copies content of fdo in input of preceeding file
        dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 

        close(fd0); // necessary
        return launch(args, fd0, shell_FG | shell_STDIN); 
    }

    //if '>' char was found in string inputted by user 
    if (out)
    {

        int fd1 ;
        if ((fd1 = creat(output , 0644)) < 0) {
            perror("Couldn't open the output file");
            exit(0);
        }           

        dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
        close(fd1);
        return launch(args, fd1, shell_FG);
        }

    //execvp(*args, args);
    return launch(args, STDOUT_FILENO, shell_FG);
}
