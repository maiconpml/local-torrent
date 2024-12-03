#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "libs/localTorrent.h"
#include "libs/directories.h"

GlobalFiles globalFiles= {NULL,NULL};
int blockSize = 0;
int amountBlocks = 0;
int amountClients = 0;
List globalRequestsList = {0,NULL};

// SERVER GLOBALS
int amountServedClient;
pthread_mutex_t globalServerMutex;
pthread_cond_t globalServerCond;

int main(int argc, char* argv[]){

    if(argc < 2){
        printf("Poucos argumentos na execução do programa. Digite ./localTorrent --help para instruções de uso.\n");
        return 0;
    }else{
        if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "help") || !strcmp(argv[1], "-help")){
        printf("Instruções de uso LocalTorrent:\n\n");
        printf("./localTorrent [NUM_USERS] [NUM_FILES] [BLOCK_SIZE] [NUM_BLOCKS]\n\n\n");
        printf("NUM_USERS\n\n    Número de diretórios que compartilharão arquivos entre si.\n\n\n");
        printf("NUM_FILES\n\n    Número de arquivos que serão compartilhados entre os usuários.\n\n\n");
        printf("BLOCK_SIZE\n\n    Tamanho de um fragmento do buffer de leitura e escrita.\n\n\n");
        printf("NUM_BLOCKS\n\n    Numero de fragmentos do buffer de leitura e escrita.\n\n\n");
        printf("O programa deve estar no mesmo nível de uma pasta chamada 'diretorios' que contém as pastas dos usuários que devem ser nomeadas como UX. Sendo que X deve variar de 1 até a quantidade de usuários passada como parâmetro.\n\n");
        return 0;

        }else if(argc < 5){
            printf("Poucos argumentos na execução do programa. Digite ./localTorrent --help para instruções de uso.\n");
            return 0;
        }else if(argc > 5){
            printf("Muitos argumentos na execução do programa. Digite ./localTorrent --help para instruções de uso.\n");
            return 0;   
        }
    }
    
    // globals
    amountClients = atoi(argv[1]);
    int numberOfFiles = atoi(argv[2]);
    blockSize = atoi(argv[3]);
    amountBlocks = atoi(argv[4]);
    amountServedClient = 0;

    pthread_t thread[amountClients];
    struct UserArgs userArgs[amountClients];

    //global list
    list_init(&globalRequestsList);

    //global files list
    generateGlobalList(amountClients, numberOfFiles);
    
    // global server
    pthread_cond_init(&globalServerCond, NULL);
    pthread_mutex_init(&globalServerMutex,NULL);
    
    for(int i=0; i<amountClients; ++i){

        userArgs[i].id = i+1;
        userArgs[i].totalFiles = numberOfFiles;
        pthread_create(&thread[i], NULL, user, (void*)&userArgs[i]);
    }

    for(int i=0; i<amountClients; ++i){

        pthread_join(thread[i], NULL);
    }

    global_list_destroy(numberOfFiles);
    list_destroy(&globalRequestsList);

    pthread_cond_destroy(&globalServerCond);
    pthread_mutex_destroy(&globalServerMutex);
    printf("\n");
    return 0;
}