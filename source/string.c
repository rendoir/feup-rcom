#include "string.h"

int init_string(String *string, char *array, size_t size)
{
    string = malloc(sizeof(String));
    if (string == NULL)
    {
        perror("Error creating String struct");
        return -1;
    }
    string->buffer = array;
    string->allocatedSpace = size;
    return 0;
}

int doubleMemoryAllocated(String *string)
{
    size_t newSize = (*string).allocatedSpace * 2;
    string->buffer = (char *)realloc(string->buffer, newSize);
    if ((*string).buffer == NULL)
    {
        perror("Error reallocating memory for string");
        return -1;
    }
    (*string).allocatedSpace = newSize;
    return 0;
}

void destroy(String *string)
{
    free(string->buffer);
    free(string);
}
