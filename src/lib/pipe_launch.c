#include<stdlib.h>
#include<unistd.h>
#include "../headers/constants.h"
#include "../headers/launch.h"

int pipe_launch(char **arg1, char **arg2)
{
    int fd[2], pid;
    pipe(fd);
    int stdin_copy = dup(STDIN_FILENO);
    
    if ((pid = fork()) == 0) {
        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[0]);
        launch(arg1, STDOUT_FILENO, shell_FG);
        exit(EXIT_FAILURE);
    }

    else if (pid > 0) {
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[1]);
        launch(arg2, STDOUT_FILENO, shell_FG);
        dup2(stdin_copy, STDIN_FILENO);
        return 1;
    }
}


