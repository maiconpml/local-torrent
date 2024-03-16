#ifndef LIST_DIR
#define LIST_DIR

typedef struct{
    char** globalFileNames;
    int * fileSizes;
}GlobalFiles;

extern GlobalFiles globalFiles;

int* listFiles(int id, int n_files);

int checkFiles(int * booleanFileList, int n_files);

void generateGlobalList(int nUsers, int nFiles);

void makeDirName(int id, char* dirName);

void files_list_destroy(char** filesList, int nFiles);

void global_list_destroy(int nFiles);

#endif