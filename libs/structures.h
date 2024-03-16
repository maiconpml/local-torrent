#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>
#include <pthread.h>

extern int blockSize;
extern int amountBlocks;
extern int amountClients;

// SERVER GLOBALS
extern int amountServedClient;
extern pthread_mutex_t globalServerMutex;
extern pthread_cond_t globalServerCond;
struct Buffer{
    int fileIndex;
    int userId;
    char * * blocks;

    // index
    int amountIndexBuffer;
    int fullBlocksQtt;
    int firstEmpty;
    long long jump; // directly 
    int sizeOfFile;
    int amountThreadsServer;

    // file
    FILE * filePointer;

    // mutex
    pthread_mutex_t mutex;
    
    // cond
    pthread_cond_t wakeUpAttendRequest;
    pthread_cond_t wakeUpDumpBuffer;
};

typedef struct{
    int indexFile;
    struct Buffer* buffer;

    pthread_t** thread;
    int serversQtt;
}GlobalListEntry;

#endif