#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include <stdio.h>

/**
 * String struct that holds a buffer and the size of the allocated memory.
 */
typedef struct String
{
    char *buffer;
    size_t allocatedSpace;
} String;

/**
 * Initializes a String.
 * <param string> The String to initialize </param>
 * <param array> Buffer associated with the String </param>
 * <param size> Initial size of the memory allocated to the array.
 * <return> 0 if success, -1 if error allocating memory for the struct</return>
 */
int init_string(String *string, char *array, size_t size);

/**
 * Doubles the allocated memory for the buffer
 * <param string> The string which buffer memory size should be doubled </param>
 */
int doubleMemoryAllocated(String *string);

/**
 * Frees the memory of the buffer of the string. Frees the string itself.
 * <param string> String which memory should be freed.
 */
void destroy(String *string);

#endif
