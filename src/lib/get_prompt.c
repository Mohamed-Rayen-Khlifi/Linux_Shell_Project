#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <errno.h>
#include "../headers/constants.h"

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
        strcpy(prompt, YELLOW"");
        strcat(prompt, user);
        strcat(prompt, WHITE"@");
        strcat(prompt, YELLOW"");
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
