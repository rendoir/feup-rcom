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

unsigned char getBCC2(char *data, int data_size)
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
  *frame_size = data_size + 6;
  (*frame) = (char *)malloc(*frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = ((sequence_number++) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC2(data, data_size);
  (*frame)[5 + data_size] = FLAG;
  byteStuffing(frame, frame_size);
}

void byteStuffing(char **frame, int *frame_size)
{
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
}

void byteUnstuffing(char **frame, int *frame_size)
{
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
}

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
