#include "linkLayer.h"

int main()
{
  return 0;
}

long sequence_number = 0;

unsigned char getAddress(int caller, char control_field)
{
  if (control_field == C_SET || control_field == C_DISC)
  {
    if (caller == SENDER)
    {
      return A_SENDER_COMMAND;
    }
    return A_RECEIVER_COMMAND;
  }
  else
  {
    if (caller == RECEIVER)
    {
      return A_RECEIVER_COMMAND;
    }
    return A_SENDER_COMMAND;
  }
}

unsigned char getBCC(char *data, int data_size)
{
  unsigned char bcc = 0;
  int i;
  for (i = 0; i < data_size; i++)
  {
    bcc = bcc ^ data[i];
  }
  return bcc;
}

void buildControlFrame(char *frame, int caller, char control_field)
{
  frame[0] = FLAG;
  frame[1] = getAddress(caller, control_field);
  frame[2] = control_field;
  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG;
}

void buildDataFrame(char **frame, char *data, int data_size, int *frame_size)
{
  printf("\nDEBUG: STRAT BUILDDATAFRAME\n");
  *frame_size = data_size + 6;
  (*frame) = (char *)malloc(*frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = ((sequence_number++) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC(data, data_size);
  (*frame)[5 + data_size] = FLAG;

  int i = 0;
  for(i = 0; i < *frame_size; i++){
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }
  byteStuffing(frame, frame_size);
  for(i = 0; i < *frame_size; i++){
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }
  printf("\nDEBUG: END BUILDDATAFRAME\n");
}

void byteStuffing(char **frame, int *frame_size)
{
  printf("\nDEBUG: START BYTESTUFFING\n");
  int i;
  int allocated_space = *frame_size;
  for (i = 1; i < *frame_size - 1; i++)
  {
    char currentByte = (*frame)[i];
    if (currentByte == FLAG || currentByte == ESCAPE)
    {
      (*frame)[i] = currentByte ^ STUFF_XOR;
      if (allocated_space <= *frame_size)
      {
        allocated_space = 2 * allocated_space;
        realloc(*frame, allocated_space * sizeof(char));
      }
      insertValueAt(ESCAPE, *frame, i, frame_size);
      i++;
    }
  }
  printf("\nDEBUG: END BYTESTUFFING\n");
}

void byteUnstuffing(char **frame, int *frame_size)
{
  printf("\nDEBUG: START BYTEUNSTUFFING\n");
  int i;
  for (i = 0; i < *frame_size; i++)
  {
    if ((*frame)[i] == ESCAPE)
    {
      char byte_xored = (*frame)[i + 1] ^ STUFF_XOR;
      if (byte_xored == FLAG || byte_xored == ESCAPE)
      {
        removeValueAt(*frame, i, frame_size);
        (*frame)[i] = byte_xored;
      }
    }
  }
  // Realloc to free unused memory
  realloc(*frame, *frame_size * sizeof(char));
  printf("\nDEBUG: END BYTEUNSTUFFING\n");
}

/*------------------------------------*/
/*------------------------------------*/
/*---------------LLOPEN---------------*/
/*------------------------------------*/
/*------------------------------------*/


int llopen(char *port, int caller){
  printf("\nDEBUG: START LLOPEN\n");
  int fileDescriptor = openSerialPort(port, caller);
  if(fileDescriptor < 0){
    return -1;
  }
  if(setNewSettings(fileDescriptor, caller) < 0){
    return -1;
  }

  int returnValue;
  if (caller == TRANSMITTER) {
    returnValue = llopenTransmitter(fileDescriptor);
  } else if (caller == RECEIVER) {
    returnValue = llopenReceiver(fileDescriptor);
  }

  printf("\nDEBUG: END LLOPEN\n");
  return returnValue;
}

int llopenSender(int fileDescriptor){
  printf("\nDEBUG: START LLOPENSENDER\n");
  char frame[5];
  buildControlFrame(frame, TRANSMITTER, C_SET);

  printf("\nDEBUG: END LLOPENSENDER\n");
  return 0;
}

int llopenReceiver(int fileDescriptor){
  printf("\nDEBUG: START LLOPENRECEIVER\n");
  char frame[5];
  buildControlFrame(frame, RECEIVER, C_UA);

  printf("\nDEBUG: END LLOPENRECEIVER\n");
}