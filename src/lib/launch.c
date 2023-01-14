#include<wait.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include "../headers/constants.h"

int launch(char **args, int fd, int options)
{

    int shell_bg = (options & shell_BG) ? 1 : 0;
    int shell_stdout = (options & shell_STDOUT) ? 1 : 0;
    int shell_stderr = (options & shell_STDERR) ? 1 : 0;
    int shell_stdin = (options & shell_STDIN) ? 1 : 0;

    pid_t pid, wpid;

    int status;

    if ((pid = fork()) == 0)
    {
 
       if (execvp(args[0], args) == -1)
        {
            fprintf(stderr, RED "Unable to execute command '%s': %s\n" RESET, args[0],strerror(errno));
        }        exit(EXIT_FAILURE);

    }

    else if (pid < 0)
    {
        fprintf(stderr, RED "Error: Child processes' forking has failed  %s\n" RESET, strerror(errno));
    }

    else
    {
        // ignore SIGINT in parent process when child process is launched
        signal(SIGINT, SIG_IGN);
        do
        {
            if (!shell_bg)
            {
                // Wait for child pid to die
                wpid = waitpid(pid, &status, WUNTRACED);
            }
            else
            {
                printf(YELLOW "This process is now launched in the background [pid: %d] - %s\n" RESET, pid, args[0]);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        // Didnt receive an exist or a signal
    }

    dup2(STDOUT_FILENO, 1);
    dup2(STDIN_FILENO, 1);
    dup2(STDERR_FILENO, 1);
    return 1;
}

