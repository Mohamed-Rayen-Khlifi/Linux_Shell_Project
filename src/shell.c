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

char *history_path = NULL;

/* The main loop of the shell */
void main_loop(void)
{

    signal(SIGINT, shell_exit_on_SIGINT);
    char *line;
    char **args;

    int status, index;

    // get prompt
    char *prompt;

    char *homedir = getenv("HOME");

    size_t history_path_len = sizeof(char) * PATH_MAX;
    history_path = malloc(history_path_len);
    snprintf(history_path, history_path_len, "%s/%s", homedir, HISTFILE);

    // read from history file
    read_history(history_path);

    do
    {
        // use GNU's readline() function
        if ((prompt = get_prompt()) == NULL)
        {
            status = 0;
            fprintf(stderr, RED "shell: Failed to get prompt.\n" RESET);
        }
        line = readline(prompt);

        // add to history for future use
        add_history(line);

        // EOF
        if (!line)
        {
            status = 0;
        }
        else
        {
            // write to history file
            write_history(history_path);

            char *history_copy;

            if (line[0] == '!' && line[1] == '-')
            {
                if (sscanf(line, "!-%d", &index) != EOF)
                {
                    HIST_ENTRY *hist_entry = history_get(history_length - index);
                    history_copy = malloc(strlen(hist_entry->line) + 1);
                    strcpy(history_copy, hist_entry->line);
                    args = split_line(history_copy);
                }
                else
                    fprintf(stderr, RED "shell: Expected digit after '!-' for history recollection.\n." RESET);
            }

            else if (line[0] == 'q')
            {
                exit(0);
                free(prompt);
            }

            else if (line[0] == '!')
            {
                if (sscanf(line, "!-%d", &index) != EOF)
                {
                    HIST_ENTRY *hist_entry = history_get(index);
                    history_copy = malloc(strlen(hist_entry->line) + 1);
                    strcpy(history_copy, hist_entry->line);
                    args = split_line(history_copy);
                    free(history_copy);
                }
                else
                    fprintf(stderr, RED "shell: Expected digit after '!' for history recollection.\n" RESET);
            }
            else
            {
                args = split_line(line);
            }

            if (args)
            {
                status = execute(args);
                free(args);
            }
            else
            {
                status = 1;
            }
        }

        free(prompt);
        free(line);

    } while (status);

    free(history_path);
}

int main(int argc, char *argv[])
{
    // Here
    int batchMode = 0;
    char *fileToRead = "/no/such/file";
    char *cmds[1000];
    char buffer[1000] = "";
    char *tmp;
    int num_cmds, i, flag, rc = 0;

    if (argc > 2)
    {

        char error_message[150] = RED "Please specify one file. \n";

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