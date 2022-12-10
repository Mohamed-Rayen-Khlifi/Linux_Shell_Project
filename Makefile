CC=gcc
LIBS=-lreadline

all: shell 

shell: shell.c 
	$(CC) $^ -o $@ $(LIBS)

clean:
	- rm shell 

.PHONY: all test clean
