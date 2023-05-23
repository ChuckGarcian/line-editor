#include "buffer.h"
#define DEBUG
int debug;
FILE * f;

void resize(ldBuffer * ldBuff);

/*Adds character to buffer at the given index*/
void insertChar(ldBuffer *ldBuf, char c, int index) {
  // if the buffer is full we need to resize

  if (ldBuf->endIndex == ldBuf->capacity - 2) {
    resize(ldBuf);
  }

  #ifdef DEBUG
   fprintf(f, "Before inesert: %s \n", ldBuf->strBuf);
  #endif

  // shift elements at pos and greater down 1 spot in array
  for (int i = ldBuf->endIndex; i > index; i--) {
    ldBuf->strBuf[i] = ldBuf->strBuf[i-1];
  }
  #ifdef DEBUG
  fprintf(f, "after: %s \n", ldBuf->strBuf);
  #endif
  // now we put the give char in the freed up slot
  ldBuf->strBuf[index] = c;
  ldBuf->endIndex++;
}

/*initiates and returns a pointer to an ld buffer; takes initial capacity*/
void initLDBuff(ldBuffer ** ldb, int initCapacity) {
    #ifdef DEBUG
    f = fopen("log1.txt", "w");
    setbuf(f, NULL);
    #endif
    *ldb = malloc(sizeof(ldBuffer));
    (*ldb)->strBuf = calloc(1, initCapacity);
    (*ldb)->capacity = initCapacity;
    (*ldb)->endIndex = 0;
    #ifdef DEBUG
    fprintf(f, "Before: %s \n", (*ldb)->strBuf);
    #endif
    
}

/*Removes the char at the given index*/
void removeChar(ldBuffer * ldBuf, int index) {
    char * curElement = ldBuf->strBuf; // pointer to the current element 

    fprintf(f, "Before: %s \n", curElement);
    fprintf(f, "indx: %d \n", index);
    fprintf(f, "endIndex: %d \n", ldBuf->endIndex);
    // Now we iterate throught the array and shift forward one starting at the given index
    for (int i = index; i < ldBuf->endIndex; i++) {
        curElement[i] = curElement[i + 1];
        fprintf(f, "durin: %s \n", curElement);
    }
     ldBuf->endIndex--;

   // ldBuf->strBuf[ldBuf->endIndex] = '\0';
    

}


/*The given pointer will be made to point to this buffers*/
// char * toString(ldBuffer * ldBuf) {
//    // ldBuf->strBuf[ldBuf->endIndex + 1] = '\0';

//     return strBuf;
// }

void resize(ldBuffer * ldBuf) {
    // Create a new character buffer
    char * newCharBuffer = malloc(ldBuf->capacity * 2);

    // copy over to the new one and free up oldone
    memcpy(newCharBuffer, ldBuf->strBuf, ldBuf->capacity);
    free(ldBuf->strBuf);

    //Update fields
    ldBuf->strBuf = newCharBuffer;  
    ldBuf->capacity =  ldBuf->capacity * 2;
}

int size(ldBuffer * ldBuf) {
    return ldBuf->endIndex;
}