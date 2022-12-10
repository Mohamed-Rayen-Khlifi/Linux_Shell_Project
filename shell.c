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
#include "constants.h"

char *history_path = NULL;


/*
##############################################################################################################

                          S   H   E   L   L   #####    B   U   I   L   T   I   N   S

##############################################################################################################


*/
/* Defining Shell builting utilities */
int shell_cd(char **args);
int shell_help(char **args);
int shell_history(char **args);

/* Function pointers that correspond to the buitins */
int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_history,
};

/* Returns the number of builtin commands available for shell */
int num_builtins()
{
    return sizeof(builtins) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == NULL || strcmp(args[1],"~")==0)
    {
        // If we write no path (only 'cd'), then go to the home directory
        chdir(getenv("HOME"));
        return 1;
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            fprintf(stderr, RED "shell: %s\n" RESET, strerror(errno));
        }
    }

    return 1;
}

/* Function that shows the help message */
int shell_help(char **args)
{
    int i;
    printf(BOLD "\nshell: A minimalistic shell written in C."
                "\n" RESET
                "Builtin commands:\n"
                "cd - Change dicrectory\n"
                "history - Show history from ~/.shell_history\n"
                "q- Exit this shell"
                "help - Show this help\n\n");

    return 1;
}

void shell_exit_on_SIGINT(int signal)
{
    exit(0);
}

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

/* Function that shows the shell history file along with line numbers
 *
 * Uses the 'cat' tool with '-n' argument to display line numbers
 */
int shell_history(char **args)
{
    char *history_args[4] = {"cat", "-n", history_path, NULL};
    return launch(history_args, STDOUT_FILENO, shell_FG);
}

int pipe_launch(char **arg1, char **arg2)
{
    int fd[2], pid;

    pipe(fd);

    int stdin_copy = dup(STDIN_FILENO);

    if ((pid = fork()) == 0)
    {
        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[0]);
        launch(arg1, STDOUT_FILENO, shell_FG);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[1]);
        launch(arg2, STDOUT_FILENO, shell_FG);
        dup2(stdin_copy, STDIN_FILENO);
        return 1;
    }
}

/* Function responsible for parsing the arguments */
int execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        return 1;
    }

    i = 0;
    while (args[i])
    {
        i++;
    }

    /* launch process in background */
    if (!strcmp("&", args[--i]))
    {
        args[i] = NULL;
        return launch(args, STDOUT_FILENO, shell_BG);
    }
    
    for (i = 0; i < num_builtins(); i++)
    {
        // strcmp("a","a") == 0
        if (!strcmp(args[0], builtins[i]))
        {
            return (*builtin_func[i])(args);
        }
    }

    int j = 0;

    while (args[j] != NULL)
    {
        // for `>` operator for redirection (stdout)
        if (!strcmp(">", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG);
        }
        // for `>>` operator for redirection (stdout with append)
        else if (!strcmp(">>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "a+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDOUT);
        }
        // for `2>` operator for redirection (stderr)
        else if (!strcmp("2>", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR);
        }
        // for `>&` operator for redirection (stdout and stderr)
        else if (!strcmp(">&", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "w+"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDERR | shell_STDOUT);
        }
        // for `<` operator for redirection (stdin)
        else if (!strcmp("<", args[j]))
        {
            int fd = fileno(fopen(args[j + 1], "r"));
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDIN);
        }
        // for piping
        else if (!strcmp("|", args[j]))
        {
            char **arg2;
            int i = 0;
            args[j] = NULL;
            arg2 = &args[j + 1];

            return pipe_launch(args, arg2);
        }
        j++;
    }

    return launch(args, STDOUT_FILENO, shell_FG);
}

/* Function that splits the line based on delimeters defined in constants.h */
char **split_line(char *line)
{
    char **tokens = malloc(sizeof(char *) * TOKEN_BUFSIZE);
    char *token;

    int bufsize_copy = TOKEN_BUFSIZE;
    int pos = 0;

    if (!tokens)
    {
        fprintf(stderr, RED "shell: Memory allocation failed." RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIMS);
    while (token != NULL)
    {
        tokens[pos] = token;
        pos++;

        if (pos >= bufsize_copy)
        {
            bufsize_copy = bufsize_copy * 2;
            tokens = realloc(tokens, sizeof(char *) * bufsize_copy);

            if (!tokens)
            {
                fprintf(stderr, RED "shell: Memory allocation failed." RESET);
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOKEN_DELIMS);
    }

    tokens[pos] = NULL;
    return tokens;
}

/* Function that returns the prompt */
char *get_prompt(void)
{
    char *prompt, tempbuf[PATH_MAX];

    char *user = getenv("USERNAME");
    char *host = getenv("DESKTOP_SESSION");
    
    size_t prompt_len = strlen(user) + strlen(host) + strlen(tempbuf)+ 1500; 

    prompt = malloc(prompt_len);

    if( getcwd( tempbuf, sizeof(tempbuf) ) != NULL ) {

        snprintf(prompt, prompt_len, "%s %% ", tempbuf);
        strcpy(prompt, BLUE"");
        strcat(prompt, user);
        strcat(prompt, WHITE"@");
        strcat(prompt, BLUE"");
        strcat(prompt, host);
        strcat(prompt, YELLOW" ");
        strcat(prompt, tempbuf);
        strcat(prompt, WHITE"");
        strcat(prompt, " %");
        strcat(prompt, " ");
        return prompt;
    }
    else {
        free(prompt);
        return NULL;
    }
}

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



void welcomeScreen()
{
    printf("\n\t============================================================\n");
    printf(RED"\t       Advanced Operating Systems Project: Basic Shell \n");
    printf(YELLOW"\t             Made by Malak, Maram and Rayen\n");
    printf(WHITE"\t============================================================\n");
    printf("\n\n");
}


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

int breakString(char *str)
{
       char *tmp;
       char *subcmds[1000];
       char buffer[1000];
       strcpy(buffer, str);
       tmp = strtok(buffer,";");
       int num_subcmds = 0;
        
       while (tmp) 
       {
          subcmds[num_subcmds] = tmp;
          num_subcmds++;
          tmp = strtok(NULL,";");
       } 


       int j, status;
           
       for (j = 0; j < num_subcmds; j++) 
       {
	 int ret;
		    if((subcmds[j][0]=='c' && subcmds[j][1]=='d') == 1)
	   {
	     breakCommand(subcmds[j]);
	   }
	 else if(strcmp(subcmds[j],"exit") == 0)
	   {
	     breakCommand(subcmds[j]);
	   }
	 else
	   {
	     if( (ret=fork()) > 0 )
	       {
                 
		 waitpid(ret,&status,WUNTRACED);
		 int x = WEXITSTATUS(status);
		 if(x==101)
		   return 101;
	       }
	     else if ( ret == 0 )
	       {
		 if(breakCommand(subcmds[j])==1)  
		   { 
		     exit(1);
		     return 101; 
		   }
		 else return 0;
		 break;
	       }
	     else 
	       {
		 char error_message[30] = "An error has occurred\n";
		 write(STDERR_FILENO, error_message, strlen(error_message));
		 exit(101);
	       }
	   }
       }
}


int main(int argc, char *argv[])
{
//Here
    int batchMode=0;
    char *fileToRead = "/no/such/file";
    char *cmds[1000];
    char buffer[1000]="test";
    char * tmp;
    int num_cmds, i, flag, rc=0;

    if (argc > 2 ) {

        char error_message[150] = RED "Please specify one file. \n" ;

	    write(STDERR_FILENO, error_message, strlen(error_message));
	    exit(1);
    }
    else if (argc == 2) {
        batchMode = 1;	        
	    fileToRead = argv[1];
    }
    else {
        welcomeScreen();
        main_loop();
    }

    FILE *new;
    FILE *fp;

    if (batchMode == 1){
        fp = fopen(fileToRead,"r");
	    if (fp==NULL) 
	    {
	        char error_message[100] = RED"File does not exist.\n";
	        write(STDERR_FILENO, error_message, strlen(error_message));
	        exit(1);
	    }
	    new=fp;
    }

    else
	new=stdin;
	
      while (!feof(new)) 
      {
          num_cmds = 0;
          if(batchMode==0) write(STDOUT_FILENO, "537sh% ", strlen("537sh% ")); 
	    //printf("%s%% ",argv[0]); 
	  if(batchMode==1)
	      fgets(buffer, 1000, fp);
	  else
	      fgets(buffer, 1000, stdin);

	  int j;

	  strtok(buffer, "\n\r");

	  if(batchMode == 1 && strcmp(buffer,"xyz")!=0)
	    {
		  //printf("Command %d: \"%s\"\n",j,subcmds[j]);
		  write(STDOUT_FILENO, buffer, strlen(buffer));
		  write(STDOUT_FILENO, "\n", strlen("\n"));
		  //if(j!=num_subcmds-1) write(STDOUT_FILENO, " ", strlen(" "));
		  if(strcmp(buffer,"+")==0)
		    {
		      //printf("%s",buffer);
		      exit(0);
		    }
	    }

	  if(strcmp(buffer,"xyz")==0) exit(0);
	  
	  //printf("Buffer is: %s\n", buffer);

	  for(i=0; buffer[i]!='\0'; i++) {
	    if(buffer[i]=='+')
	      flag++;
	  }
	  
	  if(strlen(buffer)==0)
	    {
	      char error_message[30] = "An error has occurred\n";
	      write(STDERR_FILENO, error_message, strlen(error_message));
	    }

	  j = i-2;
	  int endingWithPlus = 0;
	  for(; j >= 0; j--)
	  {
	    if(buffer[j] == ' ')
	      continue;
	    else if(buffer[j] == '+')
	      {
		endingWithPlus = 1;
		break;
	      }
	    else
	      break;
	  }

	  //printf("%d\n", flag);
          tmp = strtok(buffer,"+");
           
          while (tmp) 
          {
              cmds[num_cmds] = tmp;
              num_cmds++;
              tmp = strtok(NULL,"+");
          } 

	  int numCmndsToFork = num_cmds;
	  
	  if(flag==0)
	  {
	    if((rc=breakString(buffer))==101) {
	      //printf("exiting3..."); 
	      break; exit(0);
	    }
	  }
	  else
	    {
	      
	      if(endingWithPlus == 0)
	      {
		  numCmndsToFork = num_cmds - 1;
		   if((rc=breakString(cmds[num_cmds-1]))==101)
		   {
		     //printf("exiting3..."); 
		     break; 
		     exit(0);
		   }
	      }

	      int i, status;
	      for (i = numCmndsToFork-1; i >= 0; i--) 
		{
		  //          printf("Word %d: \"%s\"\n",i,cmds[i]);
            
		  int ret;
		  if( (ret=fork()) > 0 )
		    {
		      while (1) {
			int status;
			pid_t done = waitpid(ret,&status,WUNTRACED);
			if (done == -1) {
			  if (errno == ECHILD) break; // no more child processes
			} else {
			  int x = WEXITSTATUS(status);
			  //printf("parent - main - Status: %d\n", x);

			  if (!WIFEXITED(x) || WEXITSTATUS(x) != 101) {
			    //cerr << "pid " << done << " failed" << endl;
			    exit(0);
			  }
			}
		      }
		    }
		  else if ( ret == 0 )
		    {
		      if(breakString(cmds[i])==101) 
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
	  strcpy(buffer,"xyz");
      }
      return 0;

    return EXIT_SUCCESS;
}