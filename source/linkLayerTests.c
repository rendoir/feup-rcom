#include "linkLayer.h"

int main(){
  testByteStuffing();
}


void testByteStuffing(){
  unsigned long data_size = 4;
  char data[data_size] = "ola";
  char bcc2 = 0;
  char address_field = A_SENDER_COMMAND;
  char control_field = 0;
  char bcc1 = address_field ^ control_field;
  int i;
  for (i = 0; i < data_size){
    bcc2 = bcc2 ^ data[i];
  }

  char *frame = {FLAG,address_field,control_field,bcc1,0x6f,0x6c,0x61,}
}
