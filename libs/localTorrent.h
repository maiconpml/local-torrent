#ifndef TRRT_H
#define TRRT_H

#include "structures.h"
#include "linkedList.h"
#include <stdio.h>

extern List globalRequestsList;

typedef enum{false, true} bool;

struct UserArgs{

    int id;
    int totalFiles;
};

void* user(void* args);

#endif