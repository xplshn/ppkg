#ifndef FIND_EXECUTABLES_H
#define FIND_EXECUTABLES_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char ** paths;
    size_t  size;
} ExecuablePathList;

#define FIND_EXECUTABLES_OK                  0
#define FIND_EXECUTABLES_ERROR               2
#define FIND_EXECUTABLES_ARG_INVALID         3
#define FIND_EXECUTABLES_ENV_PATH_NO_VALUE   4
#define FIND_EXECUTABLES_ALLOCATE_MEMORY_FAILED 5

int find_executables(ExecuablePathList * out, const char * commandName, bool findAll);

#endif