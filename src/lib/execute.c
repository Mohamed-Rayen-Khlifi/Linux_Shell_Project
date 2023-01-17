#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../headers/pipe_launch.h"
#include "../headers/constants.h"
#include "../headers/launch.h"
#include "../headers/builtins.h"

int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_history,
};

char *builtins[] = {
    "cd",
    "help",
    "history",
    "quit"
};

int num_builtins() {
    return sizeof(builtins) / sizeof(char *);
}

int execute(char **args) {
    
    int i=0;
    int j=0;

    if (args[0] == NULL) {
        return 1;
    }

    while (args[i]) {
        i++;
    }

    if (!strcmp("&", args[--i])) {
        args[i] = NULL;
        return launch(args, STDOUT_FILENO, shell_BG);
    }

    for (i = 0; i < num_builtins(); i++) {
        if (!strcmp(args[0], builtins[i])) {
            return (*builtin_func[i])(args);
        }
    }

    while (args[j] != NULL) {

        if (!strcmp(">", args[j])) {
            int fd = open(args[j+1],O_WRONLY | O_CREAT | O_TRUNC, 0777);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[j] = NULL;
            return launch(args, fd, shell_FG | shell_STDOUT);
        }

        else if (!strcmp("|", args[j])) {
            
            // if (!strcmp("|", args[j+1])) {       
            //    char **args1, **args2;
            //    strncpy(args1, args[0], j-1);
            //    int firstcmd_result = launch(args1, 1, shell_FG | shell_STDOUT);
            //     if (firstcmd_result) {    
            //         return launch(args1, 1, shell_FG || shell_STDOUT);
            //        // break;
            //     }
            //     else { 
            //         strncpy(args2, args[j], strnlen(&args));
            //         return launch(&args[j+2], 1, shell_FG || shell_STDOUT);
            //      }
            
            char **arg2;
            int i = 0;
            args[j] = NULL;
            arg2 = &args[j + 1];
            return pipe_launch(args, arg2);
        }

        else if (!strcmp("&", args[j])){
            if (!strcmp("&", args[j+1])){   
                // int result1= launch(args, 1, shell_FG | shell_STDOUT);
                // if (result1 != 1){ break;
                // }
                // return and_launch(args, &args[j+2]);
                // launch(&args[j+2], 1, shell_FG | shell_STDOUT);
            }
        }

        else if (!strcmp(";", args[j])){
            // char **first_args = calloc(8000, sizeof(char*));
            // char **first_args;
            // for (int t=0; t<j; t++){
            //    first_args[t] = malloc(1000*sizeof(char*));
            //   strcpy(first_args[t],args[t]);
            // }
            // return launch(first_args, 1, shell_FG | shell_STDOUT);
            // for (int t=j+1; ;t++)
        }
        j++;
    }
    return launch(args, STDOUT_FILENO, shell_FG);
}





