
Refactoring:
        -main_loop

; && and || for interactive mode
&& and || for batch mode
pipe doesnt work in batch
redirection (>, >>, 2>1) in interactive
redirection (>>, 2>1) in batch
history doesnt work for batch mode



Understand:
        snprintf(l31 main_loop.c)
        process launch macros(l21 shell.c)
        file descriptors and dup function (pipe_launch)