#include <string.h>
#include <stdio.h>
#include "linkLayer.h"

void printArray(unsigned char* array, unsigned long size){
  int i;
  for (i = 0; i < size; i++){
    printf("Array at %d = 0x%02X\n",i,array[i]);
  }
}

void testInsertValueAt(){
  unsigned long data_size = 8;
  unsigned char *data = "ola~ana";
  unsigned char* frame;
  unsigned long frame_size = buildDataFrameLINK(&frame,data,data_size,0);
  printf("Data Frame \n");
  printArray(frame,frame_size);
  printf("\nInsert value 0x00 at index 4");
  insertValueAt(0,frame,4,&frame_size);
  printf("After Insert\n");
  printArray(frame,frame_size);
}

void testByteStuffing(){
  const char *data = "ola~ana";
  unsigned long data_size = strlen(data);
  unsigned char bcc2 = 0;
  unsigned char address_field = A_SENDER_COMMAND;
  unsigned char control_field = 0;
  unsigned char bcc1 = address_field ^ control_field;
  unsigned long i;
  for (i = 0; i < data_size; i++){
    bcc2 = bcc2 ^ data[i];
  }
  unsigned long frame_size = 6 + data_size;
  unsigned char *frame = malloc(frame_size * sizeof(unsigned char));
  frame[0] = FLAG;
  frame[1] = address_field;
  frame[2] = control_field;
  frame[3] = bcc1;

  for (i = 0; i < data_size; i++ ){
    frame[4+i] = data[i];
  }
  frame[4+data_size] = bcc2;
  frame[5+data_size] = FLAG;

  byteStuffing(&frame,&frame_size);

  printArray(frame,frame_size);

}

int main(){
  //testByteStuffing();
  testInsertValueAt();
}
