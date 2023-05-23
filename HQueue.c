#include "HQueue.h"

/*
    An Hqueue is a fix sized self dequeuing queue data structure for containing
   the previus ldBuffers; Thus its called a History qeueue; This queue may only
   contain a fixed number of ldbuffers After which the beggining of the queue
   will start to be dequeued with every additional enqueue.
*/

#define capacity 10 /*Capcity of the history stack*/

#define DEBUG
#ifdef DEBUG
FILE *file;
#endif

static ldBuffer * con[capacity];
static ldBuffer * currentldBuffer; // a container for storing the current editing bufffer 
static int Hsize; 
static int endIndex;
int HQueuePointer; /*History qeueue pointer; effects the ldBuffer returned when peek is called */
static int isInitialized = 0;  // new flag to indicate whether the queue has been initialized
/*Initilizes history stack data structure*/
void initHQueue() {
    HQueuePointer = -1; //HqeuePointer is kept relative to zero 
    if (isInitialized == 0) {
          #ifdef DEBUG
        file = fopen("HqueDebugLog.txt", "w");
        setbuf(file, NULL);
        #endif 
        Hsize = 0;
        endIndex = -1;
        isInitialized = 1;
    }
}

//Moves queue pointer up
void qpUp() {
    // Only increase the pointer if the queue is not empty and 
    // it is less than the current size
    if (Hsize != 0 && HQueuePointer < Hsize - 1) {
        HQueuePointer++;
    }
}

// Moves queue pointer down
void qpDown() {
    if (Hsize != 0 && HQueuePointer != -1) {
        HQueuePointer--;
    }
}

void HupdateCurrentBuffer(ldBuffer * buf) {
    currentldBuffer = buf;
}

/*

  
  The endIndex can be thought of as the 
  b = endIndex - mod(quePointer, capacity)
  indexOfEl = mod(b, capacity)

*/

/*returns the ldBuffer at the current history queue pointer*/
ldBuffer *Hpeek() {
    if (HQueuePointer == -1) {
        return currentldBuffer;
    }
  
    /* HqeuePointer is kept relative to zero; Thus when utilizing it for peek it must be translated such 
    that it is kept relative to endIndex and we access the proper element; IE treat endIndex as a bias*/
    int biasedQueuePointer = endIndex - HQueuePointer; 
    biasedQueuePointer = biasedQueuePointer % capacity;
    biasedQueuePointer += (biasedQueuePointer < 0) ? capacity : 0;

    // fprintf(file, "EndInex: %d \n", endIndex);
    // fprintf(file,"HQueuePointer: %d \n", HQueuePointer);
    // fprintf(file,"size: %d \n", Hsize);
    //  fprintf(file,"biasedQueuePointer: %d \n", biasedQueuePointer);
    return con[biasedQueuePointer];
}

/*Takes an ldBuffer and pushes to stack*/
void Henqueue(ldBuffer *ldb) {
  if (Hsize != capacity) {Hsize++;}
    
                 // add to internal array
  endIndex = (endIndex + 1) % capacity; // update header index
  con[endIndex] = ldb;   
    fprintf(file, "EndInex: %d \n", endIndex);
    fprintf(file,"HQueuePointer: %d \n", HQueuePointer);
    fprintf(file,"size: %d \n", Hsize);
}

// /*Frees and destroys History* Buffer*/
// void destroyStack();