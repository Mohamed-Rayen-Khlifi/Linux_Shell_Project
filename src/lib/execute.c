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

        
        else if (!strcmp("|", args[j]))
        {
            // OR operator
            //if (!strcmp("|", args[j+1])){
            //    char **args1;
            //    for (int t=0; t<j; t++){
            //        strcpy(args1[t], args[t]); 
            //    }
            //    printf("TEST");
               //// for
            //int firstcmd_result = launch(args1, 1, shell_FG | shell_STDOUT);
            //if (firstcmd_result){
                //printf("entered if");
              //  return launch(args1, 1, shell_FG || shell_STDOUT);
                //break;
            //}
           // else {
             //   return launch(&args[j+2], 1, shell_FG || shell_STDOUT);
            //}
    //        }
        // Piping
            char **arg2;
            int i = 0;
            args[j] = NULL;
            arg2 = &args[j + 1];
            return pipe_launch(args, arg2);
        }

        // AND operator
        else if (!strcmp("&", args[j])){
            printf("and");
            if (!strcmp("&", args[j+1])){   
                //int result1= launch(args, 1, shell_FG | shell_STDOUT);
                //if (result1 != 1){
                 // Error
                //}
                //return and_launch(args, &args[j+2]);
                //launch(&args[j+2], 1, shell_FG | shell_STDOUT);
            }
        }

        // AND operator
        else if (!strcmp(";", args[j])){
            //printf("entered ");
            //char **first_args = calloc(8000, sizeof(char*));
            //char **first_args;
            //for (int t=0; t<j; t++){
               //first_args[t] = malloc(1000*sizeof(char*));
              // strcpy(first_args[t],args[t]);
            //}
            //printf("entered 2");
            //return launch(first_args, 1, shell_FG | shell_STDOUT);
        }
        
        j++;


    }
      
    return launch(args, STDOUT_FILENO, shell_FG);
}





