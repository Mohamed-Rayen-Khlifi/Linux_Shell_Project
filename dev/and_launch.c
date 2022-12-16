#include<stdlib.h>
#include<unistd.h>
#include <stdio.h>
#include "../headers/constants.h"
#include "../headers/launch.h"


// Executes two processes that are connected using "|"

int and_launch(char **first_arg, char **second_arg)
{
    printf("works");
    return 1;

   /* if ((pid = fork()) == 0)
    {
        launch(arg2, STDOUT_FILENO, shell_FG)
    }

    else if (pid > 0)
    {
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[1]);
        launch(arg2, STDOUT_FILENO, shell_FG);
        dup2(stdin_copy, STDIN_FILENO);
        return 1;
    } */
}
