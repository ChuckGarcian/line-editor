#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFF_H
#define BUFF_H

// the line editor buffer
typedef struct ldBuffer {
    char * strBuf; 
    int capacity;
    int startIndex;
    int endIndex;
    //Todo move the start and end index fields over here from the main program; that is better localization

} ldBuffer;

/*Adds character to buffer at the given index*/
void insertChar(ldBuffer * ldBuf, char c, int index);

/*Removes the char at the given index*/
void removeChar(ldBuffer * ldBuf, int index);

/*The given pointer will be made to point to this buffers*/
char * toString(ldBuffer * ldBuf);
/*Returns the size of the buffer*/
int size(ldBuffer * ldBuf);


/*initiates and returns a pointer to an ld buffer; takes initial capacity*/
void initLDBuff(ldBuffer ** ldb, int initCapacity);

#endif