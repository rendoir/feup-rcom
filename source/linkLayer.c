#include <stdlib.h>
#include "macros.h"

int main(){
  return 0;
}

void insertValueAt(char value, char* array, int index, int *array_size){
  int i;
  for (i = (*array_size) -1; i > index; i--){
    array[i+1] = array[i];
  }
  array[index] = value;
  (*array_size)++;
}

void removeValueAt(char* array, int index, int *array_size){
  int i;
  for (i = index; i < (*array_size) -1; i++){
    array[i] = array[i+1];
  }
  (*array_size)--;
}

void byteStuffing(char** data, int* data_size){
   int i;
   int allocated_space = *data_size;
   for (i = 0; i < *data_size; i++){
     char currentByte = (*data)[i];
     if (currentByte == FLAG || currentByte == ESCAPE){
       (*data)[i] = currentByte ^ STUFF_XOR;
       if (allocated_space <= *data_size){
         allocated_space = 2 * allocated_space;
         realloc(*data, allocated_space * sizeof(char));
       }
       insertValueAt(ESCAPE,*data,i,*data_size);
       i++;
     }
   }
}

 void byteUnstuffing(char** data, int* data_size){
   int i;
   for (i = 0; i < *data_size; i++){
     if ((*data)[i] == ESCAPE){
       char byte_xored = (*data)[i+1] ^ STUFF_XOR;
       if (byte_xored == FLAG || byte_xored == ESCAPE){
         removeValueAt(*data,i,data_size);
         (*data)[i] = byte_xored;
       }
     }
   }
   // Realloc to free unused memory
   realloc(*data, *data_size * sizeof(char));
 }
