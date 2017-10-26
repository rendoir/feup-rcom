#include "linkLayer.h"

/* Uncomment to use them
static char write_sequence_number = 0;
static char read_sequence_number = 0;
static char last_frame_accepted = 1;
*/

int flag = 0;
unsigned int currentTries = 0;

unsigned char getAddress(int caller, unsigned char control_field)
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

unsigned char getBCC(unsigned char *data, int data_size)
{
  unsigned char bcc = 0;
  int i;
  for (i = 0; i < data_size; i++)
    bcc = bcc ^ data[i];

  return bcc;
}

void buildControlFrame(unsigned char *frame, int caller, unsigned char control_field, long sequence_number)
{
  frame[0] = FLAG;
  frame[1] = getAddress(caller, control_field);

  if (sequence_number == -1)
  {
    frame[2] = control_field;
  }
  else
  {
    frame[2] = ((sequence_number % 2) << 7) + control_field;
  }

  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG;
}

void buildDataFrame(unsigned char **frame, unsigned char *data, int data_size, unsigned long *frame_size, long sequence_number)
{
  printf("\nDEBUG: STRAT BUILDDATAFRAME\n");
  *frame_size = data_size + 6;
  (*frame) = (unsigned char *)malloc(*frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = (unsigned char)((sequence_number) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC(data, data_size);
  (*frame)[5 + data_size] = FLAG;

  unsigned long i = 0;
  for (i = 0; i < *frame_size; i++)
  {
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }

  byteStuffing(frame, frame_size);
  for (i = 0; i < *frame_size; i++)
  {
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }
  printf("\nDEBUG: END BUILDDATAFRAME\n");
}

void byteStuffing(unsigned char **frame, unsigned long *frame_size)
{
  printf("\nDEBUG: START BYTESTUFFING\n");
  int i;
  int allocated_space = *frame_size;
  for (i = 4; i < *frame_size - 1; i++)
  {
    unsigned char currentByte = (*frame)[i];
    if (currentByte == FLAG || currentByte == ESCAPE)
    {
      (*frame)[i] = currentByte ^ STUFF_XOR;
      if (allocated_space <= *frame_size)
      {
        allocated_space = 2 * allocated_space;
        if (realloc(*frame, allocated_space * sizeof(unsigned char)) == NULL)
        {
          perror("Error realloc memory for byte stuffing");
        }
      }
      insertValueAt(ESCAPE, *frame, i, frame_size);
      i++;
    }
  }
  printf("\nDEBUG: END BYTESTUFFING\n");
}

void byteUnstuffing(unsigned char **frame, unsigned long *frame_size)
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
  if (realloc(*frame, *frame_size * sizeof(char)) == NULL)
  {
    perror("Error reallocating memory for byte unstuffing");
  }
  printf("\nDEBUG: END BYTEUNSTUFFING\n");
}

/*------------------------------------*/
/*------------------------------------*/
/*---------------LLOPEN---------------*/
/*------------------------------------*/
/*------------------------------------*/

int llopen(char *port, int caller)
{
  printf("\nDEBUG: START LLOPEN\n");
  int fileDescriptor = openSerialPort(port, caller);
  if (fileDescriptor < 0)
    return -1;

  if (setNewSettings(fileDescriptor, caller) < 0)
    return -1;

  int returnValue;

  if (caller == SENDER)
    returnValue = llopenSender(fileDescriptor);
  else if (caller == RECEIVER)
    returnValue = llopenReceiver(fileDescriptor);

  printf("\nDEBUG: END LLOPEN\n");
  return returnValue;
}

int llopenSender(int fileDescriptor)
{
  printf("\nDEBUG: START LLOPENSENDER\n");
  char set_frame[5];
  buildControlFrame(set_frame, SENDER, C_SET, -1);
  writeAndReadReply(fileDescriptor, set_frame, sizeof(set_frame), C_UA);

  printf("\nDEBUG: END LLOPENSENDER\n");
  return 0;
}

int llopenReceiver(int fileDescriptor)
{
  printf("\nDEBUG: START LLOPENRECEIVER\n");
  char ua_frame[5];

  buildControlFrame(ua_frame, RECEIVER, C_UA, -1);
  writeAndReadReply(fileDescriptor, ua_frame, sizeof(ua_frame), C_SET);

  printf("\nDEBUG: END LLOPENRECEIVER\n");
  return 0;
}

int readFromFileToArray(int sp_fd, unsigned char **data, unsigned long *data_size)
{
  (*data_size) = 0;
  unsigned long frame_allocated_space = 1;
  (*data) = malloc(frame_allocated_space * sizeof(unsigned char));
  unsigned char read_char;
  while (1)
  {
    read(sp_fd, &read_char, 1);
    if (read_char == FLAG)
    {
      break;
    }
    if ((*data_size) >= frame_allocated_space)
    {
      frame_allocated_space = frame_allocated_space * 2;
      if (realloc((*data), frame_allocated_space) == NULL)
      {
        perror("Error realloc memory for read data frame");
        return -1;
      }
    }
    (*data)[(*data_size)++] = read_char;
  }
  return 0;
}

int readFrameHeader(int sp_fd, Frame_Header *expected_frame_header, int isData)
{
  State state = START;
  unsigned char read_char;
  unsigned char received_address;
  unsigned char received_control;
  int isDuplicated = 0;
  int isReject = 0;
  while (state != STOP)
  {
    if (state == BCC1_OK && isData)
    {
      break;
    }
    char currentByte = read(sp_fd, &read_char, 1);
    switch (state)
    {
    case START:
    {
      if (currentByte == FLAG)
      {
        state = FLAG_REC;
      }
      break;
    }
    case FLAG_REC:
    {
      received_address = currentByte;
      if (currentByte == expected_frame_header->address_field)
      {
        state = A_REC;
      }
      else if (currentByte != FLAG)
      {
        state = START;
      }
      break;
    }
    case A_REC:
    {
      received_control = currentByte;
      char R = currentByte >> 7;
      char notR = R ^ 1;
      if (currentByte == expected_frame_header->control_field)
      {
        state = C_REC;
      }
      else if (isData)
      {
        if (currentByte == (expected_frame_header->address_field ^ 0x40))
        {
          state = C_REC;
          isDuplicated = 1;
        }
      }
      else if (currentByte == (C_REJ + (R << 7)))
      {
        state = C_REC;
        isReject = 1;
      }
      else if (currentByte == (C_REJ + (notR << 7)))
      {
        state = C_REC;
        isDuplicated = 1;
        isReject = 1;
      }
      else if (currentByte == FLAG)
      {
        state = FLAG_REC;
      }
      else
      {
        state = START;
      }
      break;
    }
    case C_REC:
    {
      if (currentByte == (received_address ^ received_control))
      {
        state = BCC1_OK;
      }
      else if (currentByte == FLAG)
      {
        state = FLAG_REC;
      }
      else
      {
        state = START;
      }
      break;
    }
    case BCC1_OK:
    {
      if (currentByte == FLAG)
      {
        state = STOP;
      }
      else
      {
        state = START;
      }
      break;
    }
    default:
    {
      printf("State Machine Reached unexpected state\n");
      return -1;
    }
    }
  }
  if (isDuplicated)
  {
    return 1;
  }
  if (isReject)
  {
    return 2;
  }
  return 0;
}

int readDataFrame(int sp_fd, Frame_Header *frame_header, unsigned char **data_unstuffed, unsigned long *data_size)
{
  int returnValue = readFrameHeader(sp_fd, frame_header, 1);
  unsigned char *data_bcc2;
  unsigned long *data_bcc2_size;
  if (returnValue == 1)
  {
    // If no errors detected or it is duplicated
    // Should trigger a receiver ready.
    flushSP(sp_fd);
    return 0;
  }
  readFromFileToArray(sp_fd, &data_bcc2, data_bcc2_size);
  byteUnstuffing(&data_bcc2, data_bcc2_size);
  (*data_size) = *data_bcc2_size;
  unsigned char received_bcc2 = data_bcc2[(*data_bcc2_size) - 1];
  (*data_unstuffed) = malloc((*data_size) * sizeof(unsigned char));
  memcpy((*data_unstuffed), data_bcc2, (*data_size));
  unsigned char calculated_bcc2 = getBCC((*data_unstuffed), (*data_size));
  if (calculated_bcc2 == received_bcc2)
  {
    return 0;
  }
  else
  {
    // Error in BCC2 -> should trigger a C_REJ
    return -1;
  }
}

int alarmHandler()
{
  printf("Alarm #%d\n", currentTries);
  flag = 1;
  currentTries++;
  return 0;
}

int processReply(int sp_fd, char address_expected, char expected_control_field)
{
  alarm(3);
  // ControlStruct control_struct;
  // return readControlFrame(sp_fd, address_expected, expected_control_field, &control_struct);
  return 0;
}

int writeAndReadReply(int sp_fd, char *frame_to_write, int frame_size, char expected_control_field)
{
  printf("\nDEBUG: START WRITEANDPROCESSREPLY\n");

  while (currentTries++ < MAX_TRIES)
  {
    write(sp_fd, frame_to_write, frame_size);
    if (processReply(sp_fd, getAddress(/*I dont know what put here*/, expected_control_field), expected_control_field) == 0)
    {
      return 0;
    }
  }
  if (currentTries >= MAX_TRIES)
  {
    printf("\nMAX TRIES REACHED\n");
    return -1;
  }
  printf("\nDEBUG: END WRITEANDPROCESSREPLY\n");
  return 0;
}

int readAndWriteReply(int sp_fd, char *frame_to_write, int frame_size, char expected_control_field)
{
  printf("\nDEBUG: START READANDPROCESSREPLY\n");

  while (currentTries++ < MAX_TRIES)
  {
    if (processReply(sp_fd, getAddress(/*I dont know what put here*/, expected_control_field), expected_control_field) == 0)
    {
      write(sp_fd, frame_to_write, frame_size);
      return 0;
    }
  }
  if (currentTries >= MAX_TRIES)
  {
    printf("\nMAX TRIES REACHED\n");
    return -1;
  }
  printf("\nDEBUG: END READANDPROCESSREPLY\n");
  return 0;
}
