#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include "../headers/break_command.h"

int breakString(char *str)
{
	char *tmp;
	char *subcmds[1000];
	char buffer[1000];
	strcpy(buffer, str);
	tmp = strtok(buffer, ";");
	int j, status, num_subcmds = 0;

	while (tmp)
	{
		subcmds[num_subcmds] = tmp;
		num_subcmds++;
		tmp = strtok(NULL, ";");
	}

	for (j = 0; j < num_subcmds; j++)
	{
		int ret;
		if ((ret = fork()) > 0)
		{
			waitpid(ret, &status, WUNTRACED);
			int x = WEXITSTATUS(status);
			if (x == 101)
				return 101;
		}

		else if (ret == 0)
		{
			if (breakCommand(subcmds[j]) == 1)
			{
				exit(1);
				return 101;
			}
			else
				return 0;
			break;
		}

		else
		{
			char error_message[30] = "Failed to fork a child\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(101);
		}
	}
}