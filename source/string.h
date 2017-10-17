#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

typedef struct String
{
    char *buffer;
    size_t allocatedSpace;
} String;

int init_string(String *string, char *array, size_t size);

int doubleMemoryAllocated(String *string);

void destroy(String *string);

#endif