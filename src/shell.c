#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "headers/constants.h"
#include "headers/break_string.h"
#include "headers/split_line.h"
#include "headers/get_prompt.h"
#include "headers/welcome_screen.h"
#include "headers/launch.h"
#include "headers/execute.h"
#include "headers/builtins.h"
#include "headers/main_loop.h"

char *history_path = NULL;

int main(int argc, char *argv[])
{
    int batchMode = 0;
    char *fileToRead = "/no/such/file";
    char *cmds[1000];
    char buffer[1000] = "";
    char *tmp;
    int num_cmds, i, flag, rc = 0;

    if (argc > 2)
    {
        char error_message[150] = RED "Please specify one file. \n" WHITE;
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    else if (argc == 2)
    {
        batchMode = 1;
        fileToRead = argv[1];
    }

    else
    {
        welcomeScreen();
        main_loop();
    }

    FILE *new, *fp;

    if (batchMode == 1)
    {
        fp = fopen(fileToRead, "r");
        if (fp == NULL)
        {
            char error_message[100] = RED "File does not exist.\n" WHITE;
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        new = fp;
    }

    else
    {
        new = stdin;
    }

    while (!feof(new))
    {
        num_cmds = 0;
        if (batchMode == 1)
        {
            fgets(buffer, 1000, fp);
        }
        else
        {
            fgets(buffer, 1000, stdin);
        }

        int j;
        strtok(buffer, "\n\r");

        if (batchMode == 1)
        {
            if (!strcmp("quit", buffer))
            {
                write(STDOUT_FILENO, buffer, strlen(buffer));
                write(STDOUT_FILENO, "\n", strlen("\n"));
            }
        }

        if (strlen(buffer) == 0)
        {
            char error_message[80] = RED "Empty file; Please specify a valid file for the batch executor. \n" WHITE;
            write(STDERR_FILENO, error_message, strlen(error_message));
        }

        tmp = strtok(buffer, ";");

        while (tmp)
        {
            if (!strcmp(tmp, "quit"))
            {
                exit(0);
            }
            cmds[num_cmds] = tmp;
            num_cmds++;
            tmp = strtok(NULL, ";");
        }

        int numCmndsToFork = num_cmds;
        int i, status;
        for (i = 0; i < numCmndsToFork; i++)
        {
            int ret;

            if ((ret = fork()) > 0)
            {
                while (1)
                {
                    int status;
                    pid_t done = waitpid(ret, &status, WUNTRACED);
                    if (done == -1)
                    {
                        if (errno == ECHILD)
                        {
                            break;
                        }
                    }
                    else
                    {
                        int x = WEXITSTATUS(status);
                        if (!WIFEXITED(x) || WEXITSTATUS(x) != 101)
                        {
                            exit(0);
                        }
                    }
                }
            }

            else if (ret == 0)
            {
                if (breakString(cmds[i]) == 101)
                {
                    exit(0);
                }
            }

            else
            {
                char error_message[80] = RED "Error: Could not fork a child process.\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
    }
    return 0;
    return EXIT_SUCCESS;
}
