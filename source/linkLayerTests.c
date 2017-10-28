#include <string.h>
#include <stdio.h>
#include "linkLayer.h"




void testInsertValueAt(){
  unsigned char* frame;
  unsigned long frame_size;
}

void testByteStuffing(){
  const char *data = "ola~maria";
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

  int frame_index;
  for (frame_index = 0; frame_index < frame_size; frame_index++){
    printf("Frame at %d = 0x%02X\n", frame_index,frame[frame_index]);
  }

}

int main(){
  testByteStuffing();
}
