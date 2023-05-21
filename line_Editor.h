#include<stdio.h>
#include <termios.h>           
#include <unistd.h>    

/**
 * Reads an entire line from stdin; stores the address of the buffer
 * containing the text in the given 'lineptr' parameter
 * 
 * SideEffect: The proccess from which this function is envoked is blocked (paused) until return; IE
 * 
*/
void get_line(char ** lineptr);