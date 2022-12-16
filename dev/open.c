#include <fcntl.h>

int main (){
    // File descriptors
    // A unique number accross a process
    // A handler of an input / output resource ~ file, stdin, stdout, stderr
    int file = open("Path_of_file",O_WRONLY | O_CREAT, 0777);
    if (file == -1){
        return 2;
    }
    dup2(file, 1);
    close(file);
    // File descriptors
    // 0 - STDIN  
    // 1 - STDOUT  (STDOUT_FILENO)
    // 2 - STDERR
    // 3 - Path_of_file
    // dup2(file,value) + close(file)"delete the duplicated stream and leave only one"
}
