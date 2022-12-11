CC=gcc
LIBS=-lreadline

all: shell 

shell: src/shell.c src/lib/break_command.c src/lib/break_string.c src/lib/split_line.c 
	$(CC) $^ -o bin/$@ $(LIBS)
	./bin/shell

clean:
	- rm bin/shell 

.PHONY: all  clean
