#include <stdio.h>
#include <time.h>
#include "macros.h"

/**
* Inserts a value at a given position of the array.
* Increments array_size by 1;
*/
void insertValueAt(unsigned char value, unsigned char *array, int index, unsigned long *array_size);

/**
* Removes a value at a given position of the array.
* Decrements array_size by 1;
*/
void removeValueAt(unsigned char *array, int index, unsigned long *array_size);

/**
* function to log the process when DEBUG flag is set
*/
void logToFile(const char *);