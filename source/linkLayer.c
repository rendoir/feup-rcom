#include "linkLayer.h"

static char write_sequence_number = 0;
static char read_sequence_number = 0;
static char last_frame_accepted = 1;

int flag = 0;
unsigned int currentTries = 0;

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

void buildControlFrame(char *frame, int caller, char control_field, long sequence_number)
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

void buildDataFrame(char **frame, char *data, int data_size, unsigned long *frame_size, long sequence_number)
{
  printf("\nDEBUG: STRAT BUILDDATAFRAME\n");
  *frame_size = data_size + 6;
  (*frame) = (char *)malloc(*frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = (char)((sequence_number) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC(data, data_size);
  (*frame)[5 + data_size] = FLAG;

  int i = 0;
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

void byteStuffing(char **frame, unsigned long *frame_size)
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
        if (realloc(*frame, allocated_space * sizeof(char)) == NULL){
          perror("Error realloc memory for byte stuffing");
        }
      }
      insertValueAt(ESCAPE, *frame, i, frame_size);
      i++;
    }
  }
  printf("\nDEBUG: END BYTESTUFFING\n");
}

void byteUnstuffing(char **frame, unsigned long *frame_size)
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
  if (realloc(*frame, *frame_size * sizeof(char)) == NULL){
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

int readControlFrame(int sp_fd, char address_expected, char expected_control_field, ControlStruct *control_struct)
{
  State state = START;
  char char_read;
  while (state != STOP && !flag)
  {
    read(sp_fd, &char_read, 1);
    switch (state) {
      case START: {
        if (char_read == FLAG) {
          state = FLAG_REC;
        } break;
      } case FLAG_REC: {
        if (char_read == address_expected) {
          state = A_REC;
          control_struct->address_field = char_read;
        } else if (char_read != FLAG) {
          state = START;
        } break;
      } case A_REC: {
        if (char_read == expected_control_field || char_read == C_REJ) {
          state = A_REC;
          control_struct->control_field = char_read;
        } else if (char_read != FLAG) {
          state = START;
        } else {
          state = FLAG_REC;
        } break;
      } case C_REC: {
        if (char_read == (control_struct->address_field ^ control_struct->control_field)) {
          state = BCC1_OK;
        } else if (char_read != FLAG) {
          state = START;
        } else {
          state = FLAG_REC;
        } break;
      } case BCC1_OK: {
        if (char_read == FLAG) {
          state = STOP;
        } else {
          state = START;
        } break;
      } default: {
        printf("Reached unexpected state\n");
        return -1;
      }
    }
  } if(state == STOP){
    return 0;
  } else{
    return -1;
  }
}

int readDataFrame(int sp_fd, char address_expected, char expected_control_field, DataStruct *data_struct)
{
  State state = START;
  char read_char;
  int isDuplicated = 0;
  int num_flags_received = 0;
  unsigned long frame_allocated_space = 7;
  unsigned long frame_size = 0;
  char *frame_received = malloc(frame_allocated_space);
  while (num_flags_received < 2)
  {
    read(sp_fd, &read_char, 1);
    if (read_char == FLAG)
      num_flags_received++;

    if (frame_size >= frame_allocated_space)
    {
      frame_allocated_space = frame_allocated_space * 2;
      if (realloc(frame_received, frame_allocated_space) == NULL) {
        perror("Error realloc memory for read data frame");
      }
    }
    frame_received[frame_size++] = read_char;
  }
  if (frame_size < DATA_FRAME_MIN_SIZE)
  {
    printf("Data frame size should not be smaller than %d bytes", DATA_FRAME_MIN_SIZE);
    return -1;
  }

  byteUnstuffing(&frame_received, &frame_size);
  data_struct->data_allocated_space = 1;
  data_struct->data = malloc(data_struct->data_allocated_space);
  unsigned long frame_index = 0;
  while (state != STOP)
  {
    char currentByte = frame_received[frame_index++];
    switch (state) {
      case START: {
        if (currentByte == FLAG) {
          state = FLAG_REC;
        } break;
      } case FLAG_REC: {
        if (currentByte == address_expected) {
          state = A_REC;
          data_struct->address_field = currentByte;
        } else if (currentByte != FLAG) {
          state = START;
        } break;
      } case A_REC: {
        if (currentByte == expected_control_field) {
          data_struct->control_field = currentByte;
          state = C_REC;
        } else if (currentByte == (expected_control_field ^ 0x40)) {
          isDuplicated = 1;
          state = C_REC;
        } else if (currentByte == FLAG){
          state = FLAG_REC;
        } else {
          state = START;
        } break;
      } case C_REC: {
        if (currentByte == (data_struct->address_field ^ data_struct->control_field)) {
          // If Bcc is correct
          state = BCC1_OK;
          data_struct->bcc1 = currentByte;
        } else if (currentByte == FLAG) {
          state = FLAG_REC;
        } else {
          state = START;
        } break;
      } case BCC1_OK: {
        if (currentByte != FLAG) {
          state = STOP;
        } else {
          state = FLAG_REC;
        } break;
      } default: {
        printf("Reached unexpected state\n");
        return -1;
      }
    }
  }
  // If execution arrives here, is because there have been no errors previously.

  //Read data and Check BCC2
  unsigned char expected_bcc2 = frame_received[frame_size - 2];
  unsigned char bcc2 = 0;
  unsigned char error_in_bcc2 = 0;
  //frame_index = 4 is the beginning of data on a data frame.
  for (frame_index = 4; frame_index < frame_size - 2; frame_index++)
  {
    if ((frame_index - 4) >= data_struct->data_allocated_space)
    {
      data_struct->data_allocated_space = 2 * data_struct->data_allocated_space;
      if (realloc(data_struct->data, data_struct->data_allocated_space) == NULL)
      {
        perror("Error reallocating memomy for data_struct->data");
      }
    }
    data_struct->data[frame_index - 4] = frame_received[frame_index];
    bcc2 = bcc2 ^ frame_received[frame_index];
  }

  free(frame_received);
  frame_received = NULL;

  if (bcc2 != expected_bcc2) {
    error_in_bcc2 = 1; }

  if (isDuplicated)
  {
    free(data_struct);
    data_struct = NULL; //Setting unused pointers to NULL is a defensive style, protecting against dangling pointer bugs.
    return 0;
  } else {
    if (error_in_bcc2)
    {
      free(data_struct);
      data_struct = NULL; //If a dangling pointer is accessed after it is freed, you may read or overwrite random memory.
      return -1;
    } else {
      return 0;
    }
  }
}

int alarmHandler(){
  printf("Alarm #%d\n", currentTries);
  flag = 1;
  currentTries++;
  return 0;
}

int processReply(int sp_fd, char address_expected, char expected_control_field){
  alarm(3);
  ControlStruct control_struct;
  return readControlFrame(sp_fd, address_expected, expected_control_field, &control_struct);
}

int writeAndReadReply(int sp_fd, char* frame_to_write, int frame_size, char expected_control_field){
	printf("\nDEBUG: START WRITEANDPROCESSREPLY\n");

	while(currentTries++ < MAX_TRIES){
		write(sp_fd,frame_to_write,frame_size);
		if (processReply(sp_fd, getAddress(/*I dont know what put here*/,expected_control_field), expected_control_field) == 0){
			return 0;
	   }
   }
	if (currentTries >= MAX_TRIES){
		printf("\nMAX TRIES REACHED\n");
    return -1;
	}
	printf("\nDEBUG: END WRITEANDPROCESSREPLY\n");
  return 0;
}

int readAndWriteReply(int sp_fd, char* frame_to_write, int frame_size, char expected_control_field){
	printf("\nDEBUG: START READANDPROCESSREPLY\n");

	while(currentTries++ < MAX_TRIES){
		if (processReply(sp_fd, getAddress(/*I dont know what put here*/, expected_control_field), expected_control_field) == 0){
  		write(sp_fd,frame_to_write,frame_size);
			return 0;
	   }
   }
	if (currentTries >= MAX_TRIES){
		printf("\nMAX TRIES REACHED\n");
    return -1;
	}
	printf("\nDEBUG: END READANDPROCESSREPLY\n");
  return 0;
}
