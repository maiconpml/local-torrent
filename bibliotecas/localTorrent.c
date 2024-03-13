#include "localTorrent.h"
#include "directories.h"
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct ServerClientArgs{

    struct UserArgs user;
    int * userFiles;
    // mutex
};

struct RequestArgs{
    int fileIndex;
    int userId;
};

struct AttendRequestArgs{
    GlobalListEntry * request;
    int usrIndex;
    int indexFile;
};

//func
int teto(float x){
    if(x - (int) x == 0) return x;
    return ++x;
}
int getMaxIndex(int sizeOfFile){
    int tetoVal = teto((float) ((sizeOfFile %(amountBlocks * blockSize))/(float) blockSize) );

    if(tetoVal) return tetoVal;
    return amountBlocks;
}

struct Buffer * initBuffer(struct RequestArgs args){
    // char buffer
    struct Buffer * b = (struct Buffer *) malloc(sizeof(struct Buffer));

    b->blocks = (char **) calloc(amountBlocks, sizeof(char * ));
    for( int i = 0; i< amountBlocks; i++){
        b->blocks[i] = (char *) calloc(blockSize,sizeof(char));
    }

    // aux indexs
    b->fullBlocksQtt = 0;
    b->firstEmpty = 0;
    b->jump = 0;

    // file
    char  aux[50];
    makeDirName(args.userId,aux);
    strcat(aux,"~");
    strcat(aux,globalFiles.globalFileNames[args.fileIndex]);

    b->filePointer = fopen(aux,"w+");
    b->sizeOfFile = globalFiles.fileSizes[args.fileIndex];
    b->amountIndexBuffer = amountBlocks;
    b->amountThreadsServer = 0;
    pthread_mutex_init(&b->mutex,NULL);

    // initialize cond
    pthread_cond_init(&b->wakeUpAttendRequest,NULL);
    pthread_cond_init(&b->wakeUpDumpBuffer, NULL);

    return b;
}

void deallocBuffer(struct Buffer * b){ 
    pthread_cond_destroy(&b->wakeUpAttendRequest);
    pthread_cond_destroy(&b->wakeUpDumpBuffer);
    pthread_mutex_destroy(&b->mutex);
    for( int i = 0; i< amountBlocks; i++){
        free(b->blocks[i]);
    }
    fclose(b->filePointer);
    free(b->blocks);
    free(b);
}

//threads

void * attendRequest(void * args){
    struct AttendRequestArgs * argM = (struct AttendRequestArgs *)args;
    char aux[50];
    makeDirName(argM->usrIndex,aux);
    strcat(aux,globalFiles.globalFileNames[argM->request->indexFile]);
    FILE * readFile = fopen(aux,"r");
    struct Buffer * buffer = argM->request->buffer;
    int firstEmpty;
    long long jump;

    pthread_mutex_lock(&(buffer->mutex));
    buffer->amountThreadsServer++;
    pthread_mutex_unlock(&(buffer->mutex));

    while(true){
        pthread_mutex_lock(&(buffer->mutex));
        firstEmpty = buffer->firstEmpty;
        jump = buffer->jump;
        if(firstEmpty == buffer->amountIndexBuffer){
            if(buffer->jump >= buffer->sizeOfFile){
                buffer->amountThreadsServer--;
            
                if(buffer->amountThreadsServer == 0){
                    pthread_cond_signal(&buffer->wakeUpDumpBuffer);
                }
                pthread_mutex_unlock(&(buffer->mutex));
                free(argM);
                fclose(readFile);
                pthread_exit(NULL);
            }

            pthread_cond_wait(&buffer->wakeUpAttendRequest,&buffer->mutex);
            pthread_mutex_unlock(&(buffer->mutex));
            continue;
        } 
        
        buffer->firstEmpty ++;
        buffer->jump += blockSize;

        //recalcula a quantidade de indices validos no buffer em caso 
        if(buffer->jump >= buffer->sizeOfFile) buffer->amountIndexBuffer = getMaxIndex(buffer->sizeOfFile);
        
        pthread_mutex_unlock(&(buffer->mutex));

        fseek(readFile, jump, SEEK_SET);
        fread(buffer->blocks[firstEmpty], sizeof(char), blockSize, readFile);
        
        pthread_mutex_lock(&(buffer->mutex));
        buffer->fullBlocksQtt++;
        if(buffer->fullBlocksQtt == buffer->amountIndexBuffer){
            pthread_cond_signal(&buffer->wakeUpDumpBuffer);
            pthread_cond_wait(&buffer->wakeUpAttendRequest,&buffer->mutex);
        }
        pthread_mutex_unlock(&(buffer->mutex));

    }                                             
}

void * server(void * args){

    struct UserArgs userArgs = ((struct ServerClientArgs*)args)->user;
    int * userFiles = ((struct ServerClientArgs*)args)->userFiles;
    while(true){
        pthread_mutex_lock(&globalServerMutex);
        if(amountClients == amountServedClient){
            pthread_mutex_unlock(&globalServerMutex);
            break;
        } 
        pthread_mutex_unlock(&globalServerMutex);

        pthread_mutex_lock(&(globalRequestsList.mutex));
        for(Node * current = globalRequestsList.headNode->next; current != globalRequestsList.headNode; current = current->next){
            if(userFiles[current->request.indexFile] && !current->request.thread[userArgs.id-1]){
                struct AttendRequestArgs* att = (struct AttendRequestArgs*) malloc(sizeof(struct AttendRequestArgs));
                att->request = &current->request;
                att->usrIndex = userArgs.id;
                att->indexFile = current->request.indexFile+1;
                current->request.thread[userArgs.id-1] = (pthread_t*) malloc(amountClients*sizeof(pthread_t));
                ++current->request.serversQtt;
                pthread_create(current->request.thread[userArgs.id-1], NULL, attendRequest, (void*)att); 
            }
        }
        pthread_mutex_unlock(&(globalRequestsList.mutex));
        pthread_mutex_lock(&globalServerMutex);
        if(amountClients != amountServedClient){
            pthread_cond_wait(&globalServerCond, &globalServerMutex);
            pthread_mutex_unlock(&globalServerMutex);
        }else{
            pthread_mutex_unlock(&globalServerMutex);
        }
    }

    pthread_exit(NULL);
}

void * dumpBuffer(void * args){
    struct Buffer * buffer = ((struct Buffer *)args);
    
    while(true){
        
        pthread_mutex_lock(&buffer->mutex);
        if(buffer->fullBlocksQtt != buffer->amountIndexBuffer){
            pthread_cond_broadcast(&buffer->wakeUpAttendRequest);
            pthread_cond_wait(&buffer->wakeUpDumpBuffer, &buffer->mutex);
            pthread_mutex_unlock(&buffer->mutex);
        }else{
            pthread_mutex_unlock(&buffer->mutex);
        }

        for(int i = 0; i < buffer->fullBlocksQtt-1; i++){
            fwrite(buffer->blocks[i], sizeof(char), blockSize, buffer->filePointer);
        }
        if(buffer->jump >= buffer->sizeOfFile){
            int lastBlockSize = buffer->sizeOfFile % blockSize;
            lastBlockSize = (lastBlockSize == 0) ? blockSize : lastBlockSize;
            fwrite(buffer->blocks[buffer->fullBlocksQtt-1], sizeof(char), lastBlockSize, buffer->filePointer);
            fflush(buffer->filePointer);
            pthread_mutex_lock(&buffer->mutex);
            
            while(buffer->amountThreadsServer != 0) {
                pthread_cond_broadcast(&buffer->wakeUpAttendRequest);
                pthread_cond_wait(&buffer->wakeUpDumpBuffer, &buffer->mutex); 
            }
            pthread_mutex_unlock(&buffer->mutex);

            pthread_exit(NULL);
        }
        
        fwrite(buffer->blocks[buffer->fullBlocksQtt-1], sizeof(char), blockSize, buffer->filePointer);
        fflush(buffer->filePointer);
        pthread_mutex_lock(&buffer->mutex);
        buffer->fullBlocksQtt = 0;
        buffer->firstEmpty = 0;
        pthread_mutex_unlock(&buffer->mutex);
    } 
}

GlobalListEntry newGlobalListEntry(int fileIndex, struct Buffer* buffer){

    GlobalListEntry entry = {fileIndex, buffer};

    entry.thread = (pthread_t**) malloc(amountClients*sizeof(pthread_t*));

    for(int i=0; i<amountClients; ++i){
        entry.thread[i] = NULL;
    }

    entry.serversQtt = 0;

    return entry; 
}

void * makeRequest(void * args){
    struct RequestArgs argsR = *(struct RequestArgs*)args; 

    struct Buffer * buffer = initBuffer(argsR); 
    buffer->fileIndex = argsR.fileIndex;
    buffer->userId = argsR.userId;
    pthread_t dumpBufferThread;
    pthread_create(&dumpBufferThread, NULL, dumpBuffer, (void *)buffer);

    GlobalListEntry entry = newGlobalListEntry(argsR.fileIndex, buffer);
    // Critical Zone
    pthread_mutex_lock(&globalRequestsList.mutex); 

    push_front(&globalRequestsList, entry);
    Node* requestNode = globalRequestsList.headNode->next;
    
    pthread_mutex_unlock(&globalRequestsList.mutex); 
    // End of Critical Zone

    pthread_cond_broadcast(&globalServerCond);
    
    pthread_join(dumpBufferThread, NULL);

    pthread_mutex_lock(&globalRequestsList.mutex);
    remove_node(&globalRequestsList, requestNode);
    pthread_mutex_unlock(&globalRequestsList.mutex); 

    deallocBuffer(buffer);
    // rename file
    char old[50] = {0};
    char dir[50] = {0};
    makeDirName(argsR.userId, dir);
    strcat(old, dir);
    strcat(old, "~");
    strcat(old, globalFiles.globalFileNames[argsR.fileIndex]);
    strcat(dir, globalFiles.globalFileNames[argsR.fileIndex]);
    int a = rename(old, dir);

    pthread_exit(NULL);
}

void * client(void * args){

    struct UserArgs userArgs = ((struct ServerClientArgs*)args)->user;
    int * userFiles = ((struct ServerClientArgs*)args)->userFiles;
    int lastFileRequested = 0;
    
    while(true){

        int nMissingFiles = checkFiles(userFiles,userArgs.totalFiles);

        if(nMissingFiles == 0 ) {
            pthread_mutex_lock(&globalServerMutex);
            amountServedClient++;
            pthread_mutex_unlock(&globalServerMutex);
            pthread_cond_broadcast(&globalServerCond);
            break;
        }
        int nRequests = rand()%nMissingFiles+1;
        int requestIndex = 0;

        pthread_t requests[nRequests];
        struct RequestArgs auxArgs[nRequests];

        while(requestIndex < nRequests){

            if(userFiles[lastFileRequested]== 0 ){
                auxArgs[requestIndex].fileIndex = lastFileRequested;
                auxArgs[requestIndex].userId = userArgs.id;
                pthread_create(&requests[requestIndex],NULL, makeRequest,(void * )&auxArgs[requestIndex]);
                ++requestIndex;
            }

            ++lastFileRequested;
        }

        for(int i = 0; i < nRequests; i++){
            pthread_join(requests[i], NULL);
            userFiles[auxArgs[i].fileIndex] = 1;
            pthread_cond_broadcast(&globalServerCond);
        }
    }

    pthread_exit(NULL);
}

void * user(void* args){

    int id = ((struct UserArgs*)args)->id;
    int totalFiles = ((struct UserArgs*)args)->totalFiles;
    pthread_t threadClient, threadServer;
    //Search file list
    int * file_list = listFiles(id, totalFiles);
    // client & server Args
    struct ServerClientArgs  sCArgs;
    sCArgs.user = *((struct UserArgs*)args);
    sCArgs.userFiles = file_list;

    //invoca thread servidor
    pthread_create(&threadServer, NULL, server, (void*)&sCArgs);
    //invoca thread cliente

    pthread_create(&threadClient, NULL, client, (void *)&sCArgs);

    pthread_join(threadServer, NULL);
    pthread_join(threadClient, NULL);

    free(file_list);
    pthread_exit(NULL);
}