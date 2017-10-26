#include "utils.h"

void insertValueAt(unsigned char value, unsigned char *array, int index, unsigned long *array_size)
{
  int i;
  for (i = (*array_size) - 1; i > index; i--)
  {
    array[i + 1] = array[i];
  }
  array[index] = value;
  (*array_size)++;
}

void removeValueAt(unsigned char *array, int index, unsigned long *array_size)
{
  int i;
  for (i = index; i < (*array_size) - 1; i++)
  {
    array[i] = array[i + 1];
  }
  (*array_size)--;
}
