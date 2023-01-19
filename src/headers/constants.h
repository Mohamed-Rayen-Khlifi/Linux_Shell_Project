#define MAX_BUFSIZE 1024 
#define PROMPT_MAXSIZE 1024 
#define TOKEN_BUFSIZE 64 
#define TOKEN_DELIMS " \t\r\n\a" 

#define HISTFILE ".shell_history"
extern char *history_path; 

#define RESET "\e[0m"
#define BOLD "\e[1m"

#define YELLOW "\e[33m"
#define RED "\e[31m"
#define BLUE "\e[34m"
#define CYAN "\e[36m"
#define GREEN "\e[32m"
#define WHITE "\e[37m"

#define shell_FG 0x01
#define shell_BG 0x02
#define shell_STDOUT 0x04
#define shell_STDERR 0x08
#define shell_STDIN 0x10
