#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include "../headers/constants.h"

int breakCommand(char *str)
{
       char *tmp, *subcmds[1000], buffer[1000], *subnew[1000];
       strcpy(buffer, str);
       tmp = strtok(buffer," \n\t");
       int num_subcmds = 0, out, redirection_bool=0;
        
       while (tmp) 
       {
          subcmds[num_subcmds] = tmp;
          num_subcmds++;
          tmp = strtok(NULL," \n\t");
       } 
	   
       int j, loc=0;

     for (j = 0; j < num_subcmds; j++) 
	 {
		
    	printf("####Executing: \"%s\"\n",subcmds[j]);

	 }
    subcmds[j] = NULL;

    if(num_subcmds > 1)
	 {
				for (j = 0; j < num_subcmds; j++) 
					{
					if(strcmp(subcmds[j], ">") == 0)
					{
					loc=j;
					redirection_bool=1;
					break;
					}
					}


				if(redirection_bool==1)
					{
					for (j = 0; j < loc; j++) 
					{
					subnew[j]=subcmds[j];
					}
					}
				
				subnew[loc]=NULL;
	 }
       
       
       if(redirection_bool==1)
	 {
	   close(STDOUT_FILENO);
	   out = open(subcmds[loc+1], O_RDWR | O_CREAT , 0666 ); 
	   dup2(out, STDOUT_FILENO);
	   execvp(subnew[0], subnew);
	   close(out);
	   return 1; //101
	 }

    else if(strcmp(subcmds[0], "quit") == 0)
	 {
	   exit(0);
	 }

       else if (execvp(subcmds[0], subcmds) < 0)
	 {
	   char error_message[30] = "Command doesn't exist \n";
	   write(STDERR_FILENO, error_message, strlen(error_message));
	   return 1;
	 }
       return 1;
}