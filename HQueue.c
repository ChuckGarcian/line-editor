#include "HQueue.h"

/*
    An Hqueue is a fix sized self dequeuing queue data structure for containing
   the previus ldBuffers; Thus its called a History qeueue; This queue may only
   contain a fixed number of ldbuffers After which the beggining of the queue
   will start to be dequeued with every additional enqueue.
*/

#define capacity 10 /*Capcity of the history stack*/

ldBuffer *con[capacity];
ldBuffer * currentldBuffer; // a container for storing the current editing bufffer 
int Hsize; 
int endIndex;
int HQueuePointer; /*History qeueue pointer; effects the ldBuffer returned when peek is called

/*Initilizes history stack data structure*/
void initHQueue() {
    HQueuePointer = -1; //HqeuePointer is kept relative to zero 
    Hsize = 0;
    endIndex = 0;
}

//Moves queue pointer up
void qpUp() {
    if (HQueuePointer < Hsize - 1) {
        HQueuePointer++;
    }
}

// Moves queue pointer down
void qpDown() {
    if (HQueuePointer != -1) {
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
    if (HQueuePointer == -1) {return currentldBuffer;}
    int biasedQueuePointer = endIndex - HQueuePointer; 
    biasedQueuePointer = biasedQueuePointer % capacity;

    return con[biasedQueuePointer];
}

/*Takes an ldBuffer and pushes to stack*/
void Henqueue(ldBuffer *ldb) {
  if (Hsize != capacity) {Hsize++;}
  endIndex = (endIndex + 1) % capacity; // update header index
  con[endIndex] = ldb;                  // add to internal array
}

// /*Frees and destroys History* Buffer*/
// void destroyStack();