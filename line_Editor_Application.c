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
#include <assert.h>

#include "EventDispatch/KeyPressEventDispatcher.h"
#include "line_Editor.h"
#include "HQueue.h"

//#define DEBUG
#ifdef DEBUG
FILE *file;
#endif
LineData *linedata;

/*
  Puts the y and x postions of the mouse into the given x and y
  variables passed as pointers
*/
int getCursor(int *x, int *y) {
  struct termios newt, oldt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  cfmakeraw(&newt);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  char buf[100];
  // Due to a race condition where a keypress may be written to the stdin stream
  // before the terminal response, we use an 'auxiliary' buffer, unhandledBuff,
  // to store any unhandled keypresses that may have been written in.
  char unhandledBuff[6];
  int i = 0;
  // queries the terminal for cursor position status
  write(STDOUT_FILENO, "\33[6n", 5);
  #ifdef DEBUG
  fprintf(file, "get CURSOR\n");
  #endif
 
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

  #ifdef DEBUG
  fprintf(file, "x: %d, y: %d \n", *x, *y);
  fprintf(file, "Return: %d \n", ret);
  #endif

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}

// Todo make it add the change to the input buffer
void handleNonControl(char c) {
  // Adds the character to editing buffer
  insertChar(linedata->ldb, c, linedata->cursIdex - linedata->initP.x);

  linedata->cursIdex++; // Move the cursor forward one
}

/*Takes part of a control sequence and handles it; only for cursor control*/
void handleControl(event event) {
  switch (event.eventType) {
  case RIGHT_ARROW: // Move cursor right
    if (linedata->cursIdex ==
        linedata->ldb->endIndex) { // don't want to move cursor back more
      return;
    } else {
      linedata->cursIdex++;
    }
    break;
  case LEFT_ARROW: // Move cursor left
    if (linedata->initP.x ==
        linedata->cursIdex) { // don't want to move cursor further more
      return;
    } else {
      linedata->cursIdex--;
    }
    break;
  case UP_ARROW: // update editing buffer to element in history Queue
    //qpUp();  //move quepointer up
    //linedata->ldb = Hpeek();
    break;
  case DOWN_ARROW:
   // qpDown();
    //linedata->ldb = Hpeek();
    break;
  case DELETE:
    if (linedata->cursIdex == linedata->ldb->endIndex) {
      return;
    }
    removeChar(linedata->ldb, (linedata->cursIdex - linedata->initP.x));
    break;
  case BACKSPACE: //Delete the char at the current cursor pos
    if (linedata->cursIdex == linedata->initP.x) {
      return;
    }
    linedata->cursIdex--;
    assert(linedata != NULL);
    assert(linedata->ldb != NULL);
    removeChar(linedata->ldb, linedata->cursIdex - linedata->initP.x); // Remove from internal buffer
  default:
    break;
  }
}

/*Terminates the line editing proccesses*/
void endLineEdit() {
  // Todo destroy buffer
  terminateDispatcher();
  write(STDOUT_FILENO, "\n", 1);
  exit(0);
}

/* Handles the nescarry updates to display when terimnal
  when the resize signal is recieved;
*/ //Todo Finish this
void terminalResizeSigHandler(int) {
  #ifdef DEBUG
  fprintf(file, "WE ARE RESIZING!\n");
  #endif

  // To update the window size values
  ioctl(STDIN_FILENO, TIOCGWINSZ, &linedata->ws);
}


/*Handles and procceses all key events*/
void handleEvents(event event) {
  switch (event.eventType) {
      case QUIT_SEQUENCE:
        endLineEdit();
        exit(1);
        break;
      case NON_CONTROL:
        handleNonControl(event.kp);
        break;
      default: // Control Char
        handleControl(event);
        break;
      }
}

/*prints the internal editing buffer to console and correctly positions cursor*/
void pushToConsole() {
  int row, col;
  // we need to translate from a 1d array cursor index to
  // a 2d array pos
  row = (linedata->cursIdex / linedata->ws.ws_col) + linedata->initP.y;
  col = linedata->cursIdex % linedata->ws.ws_col;

  #ifdef DEBUG
  fprintf(file, "ROW: %d, COL: %d \n", row, col);
  #endif

  setbuf(stdout, NULL);
  char *outputBuf = malloc(300);
  // concates together the output buffer that will be printed
  int index = 0;
  index += sprintf(outputBuf + index, "\033[?25l"); // hides the cursor
  index += sprintf(outputBuf + index, "\033[%d;%dH", linedata->initP.y,
                   linedata->initP.x); // moves it to the initial position
  index +=
      sprintf(outputBuf + index, "\033[J"); // clears screen starting at cursor
  index +=
      sprintf(outputBuf + index, linedata->ldb->strBuf); // The editing buffer
  index += sprintf(outputBuf + index, "\033[%d;%dH", row,
                   col); // move cursor to desired position
  index += sprintf(outputBuf + index, "\033[?25h"); // show cursor
  outputBuf[index] = '\0';                          // add a null terminator

  write(STDOUT_FILENO, outputBuf,
        index + 1); // The index should now contain the total num chars
  free(outputBuf);
}

void initLineEdit() {
  /*The signal called 'sigwinch' is sent to this proccess when the
  // terminal window size is changed, thus we need to relect this
  // change internaly;
  */
  struct sigaction act = {0};
  act.sa_handler = terminalResizeSigHandler;
  sigaction(SIGWINCH, &act, NULL);

#ifdef DEBUG
  file = fopen("LineEditorLog.txt", "w");
  setbuf(file, NULL);
#endif

  linedata = malloc(sizeof(LineData));

  
  getCursor(&linedata->initP.x, &linedata->initP.y);
  linedata->cursIdex = linedata->initP.x;

  
  ioctl(STDIN_FILENO, TIOCGWINSZ, &linedata->ws); // Stores terminal window size information into the struct
  // initialize the keypress event disptacher

  initHQueue(); //History queue for storing previus ldBuffers
  initDispatcher('q'); // TODO make 'q' a macro 
  
  linedata->ldb = initLDBuff(100); // Line editing buffer; all text gets stored here
  HupdateCurrentBuffer(linedata->ldb);

  linedata->ldb->endIndex = linedata->initP.x = linedata->cursIdex;
}

int main(void) {
  write(STDOUT_FILENO, "utcsh> ", 8); // to imitate utcsh
  initLineEdit();
  event event; // struct that contains key event info

  // Todo add catagory field to filter between control events and typing events
  while (1) {
    if (pollEvent(&event) == 1) {
      handleEvents(event);
      pushToConsole();      
    }
  }
}


/*
on initlization save the number of characters for 'prefix';

we have a field in the line editor attr struct that maintains the width of the
terminal ^this is changed if the terminal size changes

every move the cursor from typing checks if the position

we shall mantain 4 values related to the cursor position:

max_x the max col the cursor is allowed to be; when max_x is equal to the
terminal width it shall loop back to zero cur_x is the current col the cursor is
in;

max_y the max row that the cursor is alloeed to be; when max_x loops over to 0
max_y shall increase by one cur_y the current row of the cursor

init_x_y the initial positon of the cursor; cursor may never go any farther left
of this position
*/
