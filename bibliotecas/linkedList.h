#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "structures.h"
#include <pthread.h>

typedef struct noAux{

    struct noAux* next;
    struct noAux* prev;
    GlobalListEntry request;
}Node;

typedef struct{
    int nElements;
    Node* headNode;
    pthread_mutex_t mutex;
}List;

void list_init(List* listPtr);

void list_destroy(List* listPtr);

void push_front(List* listPtr, GlobalListEntry request);

void remove_node(List* listPtr, Node* node);

void print(List l);

#endif