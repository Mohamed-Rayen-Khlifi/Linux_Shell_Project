

#include<wait.h>
//#include<signum-generic.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
//#include<stdio2.h>
//#include<waitflags.h>
#include<errno.h>
#include "../headers/constants.h"
/*
 * Function that is responsible for launching and handling processes.
 *
 * @args:    The argument list provided to the shell. This argument list will
 *           be used to launch and execute the child process.
 * @fd:      The file descriptor to be passed, in case of a redirection operator
 *           being passed to the shell.
 * @options: A list of options being implemented to handle background processing
 *           and redirection. The options are defined in constants.h
 */
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
        // child process

        if (fd > 2)
        {

            if (shell_stdout && dup2(fd, STDOUT_FILENO) == -1)
            {
                fprintf(stderr, RED "shell: Error duplicating stream: %s\n" RESET, strerror(errno));
                return 1;
            }

            if (shell_stderr && dup2(fd, STDERR_FILENO) == -1)
            {
                fprintf(stderr, RED "shell: Error duplicating stream: %s\n" RESET, strerror(errno));
                return 1;
            }

            if (shell_stdin && dup2(fd, STDIN_FILENO) == -1)
            {
                fprintf(stderr, RED "shell: Error duplicating stream: %s\n" RESET, strerror(errno));
                return 1;
            }

            close(fd);
        }

        if (execvp(args[0], args) == -1)
        {
            fprintf(stderr, RED "shell: %s\n" RESET, strerror(errno));
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        fprintf(stderr, RED "shell: %s\n" RESET, strerror(errno));
    }
    else
    {
        // ignore SIGINT in parent process when child process is launched
        signal(SIGINT, SIG_IGN);
        do
        {
            if (!shell_bg)
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            }
            else
            {
                printf(YELLOW "[bg][%d] - %s\n" RESET, pid, args[0]);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

