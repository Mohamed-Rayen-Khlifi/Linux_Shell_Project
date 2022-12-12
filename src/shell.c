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

    // Passing two files to the batch mode
    if (argc > 2)
    {
        char error_message[150] = RED "Please specify one file. \n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    
    // Passed a file path to the shell binary
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

    FILE *new;
    FILE *fp;

    if (batchMode == 1)
    {
        fp = fopen(fileToRead, "r");
        if (fp == NULL)
        {
            char error_message[100] = RED "File does not exist.\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        new = fp;
    }

    else
        new = stdin;

    while (!feof(new))
    {
        num_cmds = 0;
        if (batchMode == 0)
            printf("Batch mode is 0");
        // write(STDOUT_FILENO, "Quit? ", strlen("Quit?"));

        // printf("%s%% ",argv[0]);
        if (batchMode == 1)
            fgets(buffer, 1000, fp);
        else
            fgets(buffer, 1000, stdin);

        int j;

        strtok(buffer, "\n\r");

        if (batchMode == 1 && strcmp(buffer, "xyz") != 0)
        {
            // printf("Command %d: \"%s\"\n",j,subcmds[j]);
            write(STDOUT_FILENO, buffer, strlen(buffer));
            write(STDOUT_FILENO, "\n", strlen("\n"));
            // if(j!=num_subcmds-1) write(STDOUT_FILENO, " ", strlen(" "));
            if (strcmp(buffer, "+") == 0)
            {
                // printf("%s",buffer);
                exit(0);
            }
        }

        if (strcmp(buffer, "xyz") == 0)
            exit(0);

        // printf("Buffer is: %s\n", buffer);

        for (i = 0; buffer[i] != '\0'; i++)
        {
            if (buffer[i] == '+')
                flag++;
        }

        if (strlen(buffer) == 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }

        j = i - 2;
        int endingWithPlus = 0;
        for (; j >= 0; j--)
        {
            if (buffer[j] == ' ')
                continue;
            else if (buffer[j] == '+')
            {
                endingWithPlus = 1;
                break;
            }
            else
                break;
        }

        // printf("%d\n", flag);
        tmp = strtok(buffer, "+");

        while (tmp)
        {
            cmds[num_cmds] = tmp;
            num_cmds++;
            tmp = strtok(NULL, "+");
        }

        int numCmndsToFork = num_cmds;

        if (flag == 0)
        {
            if ((rc = breakString(buffer)) == 101)
            {
                // printf("exiting3...");
                break;
                exit(0);
            }
        }
        else
        {

            if (endingWithPlus == 0)
            {
                numCmndsToFork = num_cmds - 1;
                if ((rc = breakString(cmds[num_cmds - 1])) == 101)
                {
                    // printf("exiting3...");
                    break;
                    exit(0);
                }
            }

            int i, status;
            for (i = numCmndsToFork - 1; i >= 0; i--)
            {
                //          printf("Word %d: \"%s\"\n",i,cmds[i]);

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
                                break; // no more child processes
                        }
                        else
                        {
                            int x = WEXITSTATUS(status);
                            // printf("parent - main - Status: %d\n", x);

                            if (!WIFEXITED(x) || WEXITSTATUS(x) != 101)
                            {
                                // cerr << "pid " << done << " failed" << endl;
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
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            }
        }
        strcpy(buffer, "xyz");
    }
    return 0;

    return EXIT_SUCCESS;
}