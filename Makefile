CC=gcc
LIBS=-lreadline

all: shell 

shell: src/shell.c src/lib/break_command.c src/lib/break_string.c src/lib/split_line.c src/lib/get_prompt.c src/lib/welcome_screen.c src/lib/launch.c src/lib/pipe_launch.c src/lib/execute.c src/lib/builtins.c src/lib/main_loop.c 
	$(CC) $^ -o bin/$@ $(LIBS)	
	./bin/shell

	
clean:
	- rm bin/shell 

# batch: src/shell.c src/lib/break_command.c src/lib/break_string.c src/lib/split_line.c src/lib/get_prompt.c src/lib/welcome_screen.c src/lib/launch.c src/lib/pipe_launch.c src/lib/execute.c src/lib/builtins.c src/lib/main_loop.c 
# 	$(CC) $^ -o bin/$@ $(LIBS)	
#     @read -p "Specify a file name for the batch mode:" FILE \
#     ./bin/@$ $(FILE)


.PHONY: all  clean
