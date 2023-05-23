#include "buffer.h"

/*
    An Hqueue is a fix sized self dequeuing queue data structure for containing the previus ldBuffers; Thus its called
    a History qeueue; This queue may only contain a fixed number of ldbuffers After which the beggining
    of the queue will start to be dequeued with every additional enqueue. 
*/

/*Initilizes history queue data structure*/
void initHQueue();

//Moves queue pointer up
void qpUp();

// Moves queue pointer down
void qpDown();

/*returns the ldBuffer at the current history queue pointer*/
ldBuffer * Hpeek();

/*Takes an ldBuffer and pushes to stack*/
void Henqueue(ldBuffer * ldb);

void HupdateCurrentBuffer(ldBuffer * buf);

// /*Frees and destroys History queue Buffer*/
// void destroyHQueue();