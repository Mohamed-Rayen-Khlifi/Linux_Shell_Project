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

char *history_path = NULL;

/* list of builtins in the shell */
char *builtins[] = {
    "cd",
    "help",
    "history",
    "exit"
};

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
    printf( "##########################################################################################################################################\n"
                GREEN"\t \t \t \t \t \t \t Help Menu"
                "\n" RESET
                "With this shell you are able to launch most of the commands in the $PATH environment variable, however it also supports 4 builtin commands:\n"
                YELLOW"cd" WHITE"- Change dicrectory\n"
                YELLOW"history" WHITE"- Show history from ~/.shell_history\n"
                YELLOW"q" WHITE"- Exit this shell\n"
                YELLOW"help" WHITE"- Show this help\n"
                "##########################################################################################################################################\n");

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



int main(int argc, char *argv[])
{
//Here
    int batchMode=0;
    char *fileToRead = "/no/such/file";
    char *cmds[1000];
    char buffer[1000]="";
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
          if(batchMode==0)  printf("Batch mode is 0");
          //write(STDOUT_FILENO, "Quit? ", strlen("Quit?")); 
          
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