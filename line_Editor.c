#include "line_Editor.h"





void get_line(char ** lineptr) {
    int pid = fork();

    if (pid < 0) {  // forking failed
        printf("FAILED\n");
        exit();
    } else if (pid == 0) { // Child proccess
        
    } else if (pid > 0) { // Parent, we return here when the getline is finished
        
    }
}



