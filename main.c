#include "line_Editor.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char * lineptr = NULL;
    //dGetLine(&cmdline);
    char *cmdline = NULL;
    int len;
    while(1){
           //getline(&cmdline, &len, stdin);
        //   printf("%s", cmdline);
        
        ldGetLine(&cmdline);
        printf("%s", cmdline);
       // printf("hello");
    }

}



