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
        // If we write no path (only 'cd'), then go to the home directory
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

/* Function that shows the help message */
int shell_help(char **args)
{
    int i;
    printf("##########################################################################################################################################\n" GREEN "\t \t \t \t \t \t \t Help Menu"
           "\n" RESET
           "With this shell you are able to launch most of the commands in the $PATH environment variable, however it also supports 4 builtin commands:\n" YELLOW "cd" WHITE "- Change dicrectory\n" YELLOW "history" WHITE "- Show history from ~/.shell_history\n" YELLOW "q" WHITE "- Exit this shell\n" YELLOW "help" WHITE "- Show this help\n"
           "##########################################################################################################################################\n");

    return 1;
}

void shell_exit_on_SIGINT(int signal)
{
    exit(0);
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

