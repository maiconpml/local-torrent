#include "directories.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void makeDirName(int id, char* dirName){

    strcpy(dirName, "diretorios/U");
    char idString[10];
    sprintf(idString, "%d/", id);
    strcat(dirName, idString);
}

int numberOfFiles(char* dirName){

    DIR* directory = opendir(dirName);
    struct dirent* d;
    int qttFiles=0;
    while((d = readdir(directory))){
        ++qttFiles;
    }

    closedir(directory);
    return qttFiles;
}

char** listDirectory(char* dirName, int nFiles){

    DIR* directory = opendir(dirName);
    struct dirent* d;

    char** filesList = (char**) calloc(nFiles, sizeof(char*));
    for(int i=0; i<nFiles; ++i){
        filesList[i] = (char*) calloc(10, 1);
    }

    rewinddir(directory);
    if(directory){
        int i=0;
        while((d = readdir(directory))){
            if(strcmp(d->d_name, ".") && strcmp(d->d_name, "..")){
                strcpy(filesList[i++], d->d_name);
            }
        }
    }

    closedir(directory);
    return filesList;
}

int findFileIndex(char * filename){
    int j =0;
    char aux[10];

    while(isdigit(filename[j+4])){
        aux[j] = filename[j+4];
        j++;
    }

    aux[j] = 0;
    return atoi(aux)-1;// index start at 0;
}

int * generateBooleanvect(char ** filesNamesList, int n_user_files, int n_files){
    int * booleanFileList = (int*) calloc(n_files, sizeof(int));
    for(int i = 0; i <n_user_files;i++){ 
        booleanFileList[findFileIndex(filesNamesList[i])] =1; 
    }
    return booleanFileList;
} 

int getSizeOfFile(char * filename){

    FILE * aux = fopen(filename, "r");
    fseek(aux, 0, SEEK_END);
    int size = ftell(aux);
    fclose(aux);

    return size;
}

void files_list_destroy(char** filesList, int nFiles){

    for(int i=0; i<nFiles; ++i){
        free(filesList[i]);
    }
    free(filesList);
}

void global_list_destroy(int nFiles){

    files_list_destroy(globalFiles.globalFileNames, nFiles);
    free(globalFiles.fileSizes);
}

void generateGlobalList(int nUsers, int nFiles){
    globalFiles.globalFileNames = (char * *) calloc(nFiles,sizeof(char*));
    globalFiles.fileSizes = (int *) calloc(nFiles,sizeof(int));

    for(int i  = 0; i<nFiles;i++) globalFiles.globalFileNames[i] = (char*) calloc(50, sizeof(char));

    for(int i =0; i<nUsers; i++){
        char dirName[25];
        makeDirName(i+1, dirName);
        int n_user_files = numberOfFiles(dirName)-2;
        char** filesNamesList = listDirectory(dirName,n_user_files);
        
        for(int j = 0; j < n_user_files; j++){
            
            int fileIndex = findFileIndex(filesNamesList[j]);
            strcpy(globalFiles.globalFileNames[fileIndex] , filesNamesList[j]);
            
            char aux[50];
            strcpy(aux,dirName);
            strcat(aux,filesNamesList[j]); 
            globalFiles.fileSizes[fileIndex] = getSizeOfFile(aux);
        }

        files_list_destroy(filesNamesList, n_user_files);
    }
}

int* listFiles(int id,int n_files){

    char dirName[25];
    makeDirName(id, dirName);
    int n_user_files = numberOfFiles(dirName)-2;
    char** filesNamesList = listDirectory(dirName,n_user_files);
    int * booleanFileList = generateBooleanvect(filesNamesList,n_user_files,n_files);
    files_list_destroy(filesNamesList, n_user_files);
    return booleanFileList;
}

int checkFiles(int * booleanFileList, int n_files){
    int sum = 0;

    for(int i = 0; i<n_files; i++){
        sum += booleanFileList[i];
    }

    return n_files - sum ;
}
