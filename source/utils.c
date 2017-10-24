#include "utils.h"

void insertValueAt(char value, char *array, int index, int *array_size)
{
  int i;
  for (i = (*array_size) - 1; i > index; i--)
  {
    array[i + 1] = array[i];
  }
  array[index] = value;
  (*array_size)++;
}

void removeValueAt(char *array, int index, int *array_size)
{
  int i;
  for (i = index; i < (*array_size) - 1; i++)
  {
    array[i] = array[i + 1];
  }
  (*array_size)--;
}
