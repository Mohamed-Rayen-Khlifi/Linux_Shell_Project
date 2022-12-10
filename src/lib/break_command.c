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
       int num_subcmds = 0, out, flag1=0;
        
       while (tmp) 
       {
          subcmds[num_subcmds] = tmp;
          num_subcmds++;
          tmp = strtok(NULL," \n\t");
       } 
            
       int j, loc=0;

     for (j = 0; j < num_subcmds; j++) 
	 {
    	     printf("Executing:\"%s\"\n",subcmds[j]);

	 }
       subcmds[j] = NULL;

       if(num_subcmds > 1)
	 {
	   for (j = 0; j < num_subcmds; j++) 
	     {
	       if(strcmp(subcmds[j], ">") == 0)
		 {
		   loc=j;
		   flag1=1;
		   break;
		 }
	     }


	   if(flag1==1)
	     {
	       for (j = 0; j < loc; j++) 
		 {
		   subnew[j]=subcmds[j];
		 }
	     }
       
	   subnew[loc]=NULL;
	 }
       
       int i, savedJ, flag2 = 0;

       if(flag1!=1)
	   for (j = 0; j < num_subcmds; j++) 
	     {
	       i = strlen(subcmds[j]) - 1;
	       if(subcmds[j][i]=='>')
		 {
		   subcmds[j][i]='\0';
		   flag2 = 1;
		   savedJ=j;
		 }
	     }

       if(flag2==1)
	 {
	   for (j = 0; j <= savedJ; j++) 
	     {
	       subnew[j]=subcmds[j];
	     }
	   subnew[savedJ+1]=NULL;
	 }
 
       
       if(flag1==1)
	 {
	   close(STDOUT_FILENO);
	   out = open(subcmds[loc+1], O_RDWR | O_CREAT , 0666 ); 
	   if(out < 0)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(0);
	     }
	   dup2(out, STDOUT_FILENO);
	   if(execvp(subnew[0], subnew) < 0)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(101);
	     }
	   close(out);
	   return 101;
	 }
       else if(flag2==1)
	 {
	   close(STDOUT_FILENO);
	   out = open(subcmds[savedJ+1], O_RDWR | O_CREAT , 0666 ); 
	   if(out < 0)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(0);
	     }
	   dup2(out, STDOUT_FILENO);
	   if(execvp(subnew[0], subnew) < 0)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(101);
	     }
	   close(out);
	   return 101;
	 }
	 else if(strcmp(subcmds[0], "cd") == 0)
	 {
	   int res;
	   if(subcmds[1]!=NULL)
	     {
	       res = chdir(subcmds[1]);
	     }
	   else
	     {
	       res = chdir(getenv("HOME"));
	     }

	   if(res == -1)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(101);
	     }
	 }
       else if(strcmp(subcmds[0], "exit") == 0)
	 {
	   exit(0);
	 }
       else if(strcmp(subcmds[0], "pwd") == 0)
	 {
	   if(subcmds[1]!=NULL)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(0);
	     }
	   else if (execvp(subcmds[0], subcmds) < 0)
	     {
	       char error_message[30] = "An error has occurred\n";
	       write(STDERR_FILENO, error_message, strlen(error_message));
	       exit(101); 
	     }
	 }
       else if (execvp(subcmds[0], subcmds) < 0)
	 {
	   char error_message[30] = "An error has occurred\n";
	   write(STDERR_FILENO, error_message, strlen(error_message));
	   return 1;
	 }
       return 1;
}