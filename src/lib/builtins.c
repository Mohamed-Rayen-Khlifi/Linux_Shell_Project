#include<string.h>
#include<unistd.h>
//#include<"stdio2.h">
#include<stdlib.h>
#include<errno.h>
#include<stdio.h>
#include "../headers/launch.h"
#include "../headers/constants.h"


int shell_cd(char **args)
{
    if (args[1] == NULL || strcmp(args[1], "~") == 0)
    {
        chdir(getenv("HOME"));
        return 1;
    }

    else
    { 
        if (chdir(args[1]) != 0)
        {
            fprintf(stderr, RED "cd command error: %s\n" RESET, strerror(errno));
        }
    }

    return 1;
}

int shell_help(char **args)
{
    printf("##########################################################################################################################################\n" GREEN "\t \t \t \t \t \t \t Help Menu"
           "\n" RESET
           "With this shell you are able to launch most of the commands in the $PATH environment variable, however it also supports 4 builtin commands:\n" YELLOW "cd" WHITE "- Change dicrectory\n" YELLOW "history" WHITE "- Show history from ~/.shell_history\n" YELLOW "quit" WHITE "- Exit this shell\n" YELLOW "help" WHITE "- Show this help\n"
           "##########################################################################################################################################\n");

    return 1;
}


/* Function that shows the shell history file along with line numbers
 *
 * Uses the 'cat' tool with '-n' argument to display line numbers
 */
int shell_history(char **args)
{
    char *history_args[4] = {"cat", "-n", history_path, NULL};
    return launch(history_args, STDOUT_FILENO, shell_FG);
}

