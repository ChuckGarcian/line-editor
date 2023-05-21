#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

// TODO make these coments shorter
typedef struct LineData {
  int cursorStartPos; // The start position that the cursor assumes with every
                      // new line; the cursor may not move any more left of this
                      // positoin
  int lineEndPos; // The end position of the current line; cursor may not move
                  // past this position; increased when keys are typed; dec when
                  // delete pressed
  char *buf; // character buffer that getLine() returns when enter is pressed

} LineData;
FILE *file;
/*Moves the cursor to the given x and y variable*/
void moveCursor(int x, int y) {
 int len = 100;
    char str[len];

    sprintf(str, "\033[%d;%dH", y, x);

    write(STDOUT_FILENO, str, strlen(str));
}

/*
  Puts the y and x postions of the mouse into the given x and y
  variables passed as pointers
*/
int getCursor(int *x, int *y) {
  char buf[100];
  int i = 0;
  // queries the terminal for cursor position status
  write(STDOUT_FILENO, "\33[6n", 5);
  // fprintf(file, "get CURSOR\n");
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

void handleDeletion(LineData *linedata) {
  // printf("  YESS");
  int x, y;
  getCursor(&x, &y);
  if (linedata->cursorStartPos == x)
    return; // don't want to move cursor back more
  x--;
  moveCursor(x, y); // Move cursor right
}

/*Takes part of a control sequence and handles it; only for cursor control*/
void handleCursorControl(char z, LineData *linedata) {
  int x, y;
  char c;
  read(STDIN_FILENO, &c, 1); // To get rid of the '['
  read(STDIN_FILENO, &c, 1); // Now we can read in which key was pressed
  getCursor(&x, &y);
  switch (c) {
    case 'C' :// Move cursor right
      if (linedata->lineEndPos == x) {return;} // don't want to move cursor back more
      x++;
      moveCursor(x, y); 
      break;
    case 'D': // Move cursor left
      if (linedata->cursorStartPos == x) {return;} // don't want to move cursor further more
      x--;
      moveCursor(x, y);
      break;
    default:
      break;
  }
}

// Sets settings back to their og state
// and exits the proccess
void endLineEdit(struct termios oldt) {
  // To restore to orignal terminal settins
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  exit(0);
}

// Todo make it add the change to the input buffer
void handleNonControlChar(char c, LineData *linedata) {
  linedata->lineEndPos++;
  write(STDOUT_FILENO, &c, 1); // Non-control characters;
}

// Todo I need to add a bufffer and then deletion powers
int main(void) {
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);

  // Todo what happens when the user types more chars than the buffer has enough
  // to hold? Todo put This in an initilizer
  LineData *linedata = malloc(sizeof(LineData));
  ;
  linedata->cursorStartPos = 5;
  linedata->lineEndPos = 5;
  linedata->buf = malloc(100);

  newt = oldt;
  char c;
  file = fopen("log.txt", "w");
  setbuf(file, NULL);

  cfmakeraw(&newt);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  int x, y;
  getCursor(&x, &y);
  moveCursor(linedata->lineEndPos, y);

  while (1) {

    read(STDIN_FILENO, &c, 1);
    // if (iscntrl(c)) {
    //   fprintf(file, "%d\n", c);
    // } else {
    //   fprintf(file, "%d ('%c')\n", c, c);
    // }
    switch (c) {
    case 'q': // End the line edit proccess
      endLineEdit(oldt);
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
      handleNonControlChar(c, linedata); // handles non control characters
      break;
    }
  };
}