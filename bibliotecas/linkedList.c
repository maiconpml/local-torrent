#include "linkedList.h"
#include <stdlib.h>

void list_init(List* listPtr){

    listPtr->headNode = (Node*) malloc(sizeof(Node));
    listPtr->headNode->next = listPtr->headNode;
    listPtr->headNode->prev = listPtr->headNode;
    listPtr->nElements = 0;

    pthread_mutex_init(&listPtr->mutex, NULL);
}

void list_destroy(List* listPtr){

    Node* currentNode;
    int size = listPtr->nElements;
    for(int i=0; i<size; ++i){
        currentNode = listPtr->headNode->next;
        remove_node(listPtr, currentNode);
    }

    pthread_mutex_destroy(&listPtr->mutex);

    free(listPtr->headNode);
}

Node* alloc_node(GlobalListEntry* request){

    Node* newNode = (Node*) malloc(sizeof(Node));
    
    newNode->request = *request;
    newNode->next = newNode;
    newNode->prev = newNode;

    return newNode;
}

void push_front(List* listPtr, GlobalListEntry request){

    Node* newNode = alloc_node(&request);

    newNode->next = listPtr->headNode->next;
    newNode->prev = listPtr->headNode;
    newNode->next->prev = newNode;
    listPtr->headNode->next = newNode;
    ++listPtr->nElements;
}

void remove_node(List* listPtr, Node* node){
    
    node->next->prev = node->prev;
    node->prev->next = node->next;
    --listPtr->nElements;

    for(int i=0; i<amountClients; ++i){
        if(node->request.thread[i]){
            pthread_join(*node->request.thread[i], NULL);
            free(node->request.thread[i]);
        }
    }

    free(node->request.thread);
    free(node);
}

void print(List l){

    printf("Requests list %d:\n", l.nElements);
    for(Node* currentNode = l.headNode->next; currentNode != l.headNode; currentNode = currentNode->next){

        printf("file%d, Now serving: %d\n", currentNode->request.indexFile+1, currentNode->request.serversQtt);
    }
}