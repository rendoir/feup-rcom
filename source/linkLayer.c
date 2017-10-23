#include "linkLayer.h"

void byteStuffing(char** data, int* data_size){
  int i;
  int allocated_space = *data_size;
  for (i = 0; i < *data_size; i++){
    char currentByte = (*data)[i];
    if (currentByte == FLAG || currentByte == ESCAPE){
      (*data)[i] = currentByte ^ STUFF_XOR;
      if (allocated_space <= *data_size){
        allocated_space = 2 * allocated_space;
        *data = realloc(allocated_space * sizeof(char));
      }
      insertValueAt(ESCAPE,*data,i,*data_size);
      (*data_size)++;
    }
  }
}

void insertValueAt(char value, char* array, int index, int array_size){
  int i;
  for (i = array_size -1; i > index; i--){
    array[i+1] = array[i];
  }
  array[index] = value;
}
