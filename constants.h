#define MAX_BUFSIZE 1024 /* Maximum size for the command buffer */
#define PROMPT_MAXSIZE 1024 /* Maximum size of the prompt */
#define TOKEN_BUFSIZE 64 /* Maximum size of each of the tokens in the command */
#define TOKEN_DELIMS " \t\r\n\a" /* token delimeters */

#define HISTFILE ".shell_history"

/* list of builtins in the shell */
char *builtins[] = {
    "cd",
    "help",
    "history",
    "exit"
};

/* terminal styles */
#define RESET "\e[0m"
#define BOLD "\e[1m"

/* terminal colors */
#define YELLOW "\e[33m"
#define RED "\e[31m"
#define BLUE "\e[34m"
#define CYAN "\e[36m"
#define GREEN "\e[32m"
#define WHITE "\e[37m"

/* process launch macros */
#define shell_FG 0x01
#define shell_BG 0x02
#define shell_STDOUT 0x04
#define shell_STDERR 0x08
#define shell_STDIN 0x10
