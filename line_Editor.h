#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

/**
 * Reads an entire line from stdin; stores the address of the buffer
 * containing the text in the given 'lineptr' parameter
 *
 * SideEffect: The proccess from which this function is envoked is blocked
 * (paused) until return; IE
 *
 */
void get_line(char **lineptr);


/*Struct for cursor position*/
typedef struct pos {
  int x;
  int y;
} cPos;

// TODO make these coments shorter
// Todo rename struct to LineEditorInfo
/*This struct should contain infromation related to the line editor*/
typedef struct LineData {
  struct termios oldt; // orignal terminal settings
  char *buf; // character buffer that getLine() returns when enter is pressed
  struct winsize ws; // For terminal window size information into the struct

  // The start position that the cursor assumes with every
  // new line; the cursor may not move any more left of this
  // positoin
  cPos initP;
  int lineEndPos;

  // The end position of the current line; cursor may not
  // move past this position; increased when keys are typed;
  // dec when delete pressed
  cPos maxP;

  // Current cursor position
  cPos curP;
} LineData;