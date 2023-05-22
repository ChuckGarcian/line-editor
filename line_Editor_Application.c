#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include "line_Editor.h"

FILE *file;
/*Moves the cursor to the given x and y variable*/
void moveCursor(int x, int y) {
  int len = 100;
  char str[len];

  // fprintf(file, "MOVE CURSOR\n");
  // fprintf(file, "x: %d, y: %d \n", x, y);

  // Escape char sequence that makes the terminal move; Look at man pages
  sprintf(str, "\033[%d;%dH", y, x);

  write(STDOUT_FILENO, str, strlen(str));
}
LineData * linedata;

/*
  Puts the y and x postions of the mouse into the given x and y
  variables passed as pointers
*/
int getCursor(int *x, int *y) {
  char buf[100];

  // Due to a race condition where a keypress may be written to the stdin stream
  // before the terminal response, we use an 'auxiliary' buffer, unhandledBuff,
  // to store any unhandled keypresses that may have been written in.
  char unhandledBuff[6];
  int i = 0;
  // queries the terminal for cursor position status
  write(STDOUT_FILENO, "\33[6n", 5);
  fprintf(file, "get CURSOR\n");
  //  Terminal sends response to stdin and we need to read it in
  while (i < sizeof(buf) - 1) {
    read(STDIN_FILENO, &buf[i], 1);
    // if (read(STDIN_FILENO, &buf[i], 1) != 1) {break;}
    if (buf[i] == 'R') {
      break;
    } // acording to man pages it should end in 'R'
    i++;
  }

  // Make it NULL terminated
  buf[i] = '\0';
  if (buf[0] != '\33' || buf[1] != '[') {
    return -1;
  }

  // Todo add error handling in the case that sscanf does not succesfully match
  // ie returns a value less than 2
  //  First two chars are characters that indicate it is an escape sequence
  int ret =
      sscanf(&buf[2], "%d;%d", y,
             x); // Finnaly we put the response into the given y and x variable

  fprintf(file, "x: %d, y: %d \n", *x, *y);
  fprintf(file, "Return: %d \n", ret);
  return 0;
}
//Todo fix it so that when deleting not at the max pos the  max position is updated correctly
void handleDeletion(LineData *linedata) {

  int col, row;
  col = linedata->curP.x;
  row = linedata->curP.y;
  if (linedata->initP.x == col && linedata->initP.y == row)
    return; // don't want to move cursor back more
  col--;
  if (col < 1) {
    col = linedata->ws.ws_col;
    row--;
  }

  moveCursor(col, row);
  linedata->curP.x = col;
  linedata->curP.y = row;
  linedata->maxP.x = col;
  linedata->maxP.y = row;
}

// Todo make it add the change to the input buffer
void handleNonControlChar(char c, LineData *linedata) {
  linedata->maxP.x++;
  linedata->curP.x++; 
  if (linedata->maxP.x > linedata->ws.ws_col) {// Greater than the terminal col; make cursor new line
    linedata->maxP.x = 1;
    linedata->curP.x = 1;
    linedata->maxP.y++;
    linedata->curP.y++;
  }
  write(STDOUT_FILENO, &c, 1); // Non-control characters;
}

/*Takes part of a control sequence and handles it; only for cursor control*/
void handleCursorControl(char z, LineData *linedata) {
  int col, row;
  char c;
  read(STDIN_FILENO, &c, 1); // To get rid of the '['
  read(STDIN_FILENO, &c, 1); // Now we can read in which key was pressed
  
  col = linedata->curP.x;
  row = linedata->curP.y;
  switch (c) {
  case 'C': // Move cursor right
    if (linedata->maxP.x == col && linedata->maxP.y == row) { // don't want to move cursor back more
      return;
    } // needs to wrap around to line below 
    else if (col == linedata->ws.ws_col && linedata->maxP.y != row) {
      col = 1;
      row++;
    } else {
      col++;
    }
  
    moveCursor(col, row);
    break;
  case 'D': // Move cursor left
    if (linedata->initP.x == col && linedata->initP.y == row) {// don't want to move cursor further more
      return;
    } // cursor needs to wrap around to end of the line above 
    else if (col == 1 && linedata->initP.y != row) {
      row--;
      col = linedata->ws.ws_col;

    } else {
      col--;
    }

   
    moveCursor(col, row);
    break;
  case '\n':
    fprintf(file, "NEW LINE");
    break;
  default:
    break;
  }

  linedata->curP.x = col;
  linedata->curP.y = row;
}

// Sets settings back to their og state
// and exits the proccess
void endLineEdit(struct termios oldt) {
  // To restore to orignal terminal settins
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  exit(0);
}



/* Handles the nescarry updates to display when terimnal
  when the resize signal is recieved; 
*/
void terminalResizeSigHandler(int) {
  fprintf(file, "WE ARE RESIZING!\n");
  // To update the window size values
  ioctl( STDIN_FILENO, TIOCGWINSZ, &linedata->ws);
}

/*
on initlization save the number of characters for 'prefix';

we have a field in the line editor attr struct that maintains the width of the terminal
^this is changed if the terminal size changes

every move the cursor from typing checks if the position 

we shall mantain 4 values related to the cursor position: 

max_x the max col the cursor is allowed to be; when max_x is equal to the terminal width it shall loop back to zero
cur_x is the current col the cursor is in;

max_y the max row that the cursor is alloeed to be; when max_x loops over to 0 max_y shall increase by one
cur_y the current row of the cursor

init_x_y the initial positon of the cursor; cursor may never go any farther left of this position
*/

void initLineEdit(LineData * linedata) {  
  struct termios newt;
  struct sigaction act = {0};
  act.sa_handler = terminalResizeSigHandler;
  
  // The signal called 'sigwinch' is sent to this proccess when the
  // terminal window size is changed, thus we need to relect this
  // change internaly;
  sigaction(SIGWINCH, &act, NULL);


  // Todo what happens when the user types more chars than the buffer has enough
  // to hold? Todo put This in an initilizer

  linedata->buf = malloc(100);

  tcgetattr(STDIN_FILENO, &linedata->oldt);

  newt = linedata->oldt;

  file = fopen("log.txt", "w");
  setbuf(file, NULL);
  
  cfmakeraw(&newt);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  
  ioctl( STDIN_FILENO, TIOCGWINSZ, &linedata->ws); // Stores terminal window size information into the struct
  
  getCursor(&linedata->initP.x, &linedata->initP.y);
  linedata->maxP.x = linedata->initP.x;
  linedata->maxP.y = linedata->initP.y;
  
  linedata->curP.x = linedata->initP.x;
  linedata->curP.y = linedata->initP.y;
}
#include <signal.h>
// Todo I need to add a bufffer and then deletion powers
int main(void) {
  write(STDOUT_FILENO, "utcsh> ", 8); // to imitate utcsh
  linedata = malloc(sizeof(LineData));
  initLineEdit(linedata);

  char c;
  while (1) {
    

    read(STDIN_FILENO, &c, 1);
    // if (iscntrl(c)) {
    //   fprintf(file, "%d\n", c);
    // } else {
    //   fprintf(file, "%d ('%c')\n", c, c);
    // }
    switch (c) {
    case 'q': // End the line edit proccess
      endLineEdit(linedata->oldt);
      break;
    case '\033':
      // write(STDOUT_FILENO, "", 0);
      // fprintf(file, "HandlingCursorControl \n");
      handleCursorControl(c, linedata);
      break;
    case 127: // Back space
      handleDeletion(linedata);
      break;
    default:
      //fprintf(file, "NEW LINE\n");
      handleNonControlChar(c, linedata); // handles non control characters
      break;
    }
  };
}