#include "linkLayer.h"

int flag = 0;

static unsigned long sequence_number = 0;

void alarmHandler(int time)
{
  printf("Alarm Interrupt\n");
  flag = 1;
}

unsigned char getAddress(int caller, unsigned char control_field)
{
  logToFile("GETADRESS: Start");
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

void buildSupervisionFrame(unsigned char *frame, int caller, unsigned char control_field, unsigned long sequence_number_local)
{
  frame[0] = FLAG;
  frame[1] = getAddress(caller, control_field);

  if (sequence_number_local == -1)
  {
    frame[2] = control_field;
  }
  else
  {
    frame[2] = ((sequence_number_local % 2) << 7) | control_field;
  }

  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG;
}

unsigned long buildInformationFrame(unsigned char **frame, unsigned char *data, int data_size, unsigned long sequence_number_local)
{
  unsigned long frame_size = data_size + 6;
  (*frame) = (unsigned char *)malloc(frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = (unsigned char)((sequence_number_local) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC(data, data_size);
  (*frame)[5 + data_size] = FLAG;
  unsigned long i = 0;
  printf("[Link Layer] Information Frame:\n");
  for (i = 0; i < frame_size; i++)
  {
    printf("%02X ", (*frame)[i]);
  }
  printf("\n");

  return frame_size;
}

void byteStuffing(unsigned char **frame, unsigned long *frame_size)
{
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
        allocated_space *= 2;
        *frame = realloc(*frame, allocated_space * sizeof(unsigned char));
        if ((*frame) == NULL)
        {
          perror("Error realloc memory for byte stuffing");
        }
      }
      insertValueAt(ESCAPE, *frame, i, frame_size);
      i++;
    }
  }
  printf("[Link Layer] Finished Byte Stuffing\n");
}

void byteUnstuffing(unsigned char **frame, unsigned long *frame_size)
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
  *frame = realloc(*frame, *frame_size * sizeof(char));
  if ((*frame) == NULL)
  {
    perror("Error reallocating memory for byte unstuffing");
  }
  printf("[Link Layer] Finished Byte Unstuffing\n");
}

int llclose(int sp_fd, int caller)
{
  if (caller == SENDER)
  {
    return llcloseSender(sp_fd);
  }
  else if (caller == RECEIVER)
  {
    return llcloseReceiver(sp_fd);
  }
  else
  {
    printf("Unexpected caller %d", caller);
    return -1;
  }
}

int llcloseSender(int sp_fd)
{
  unsigned char sender_disc_frame[5];
  unsigned char ua[5];
  buildSupervisionFrame(sender_disc_frame, SENDER, C_DISC, -1);
  buildSupervisionFrame(ua, SENDER, C_UA, -1);
  int returnValue = writeAndReadReply(sp_fd, sender_disc_frame, 5, C_DISC, SENDER);
  if (returnValue < 0)
  {
    perror("Could not close sender. Error reading DISC\n");
    return -1;
  }
  printf("Sent Disc and Received Disc\n");
  if (write(sp_fd, ua, 5) != 5)
  {
    printf("Error sending UA\n");
    return -1;
  }
  printf("Sent UA\n");
  printf("Ended connection\n");
  return 0;
}

int llcloseReceiver(int sp_fd)
{
  int returnValue;
  unsigned char receiver_disc_frame[5];
  buildSupervisionFrame(receiver_disc_frame, RECEIVER, C_DISC, -1);
  Frame_Header expected_frame_header;
  expected_frame_header.address_field = getAddress(SENDER, C_DISC);
  expected_frame_header.control_field = C_DISC;
  readFrameHeader(sp_fd, &expected_frame_header, 0);
  printf("Read DISC\n");
  returnValue = writeAndReadReply(sp_fd, receiver_disc_frame, 5, C_UA, RECEIVER);
  if (returnValue < 0)
  {
    perror("Could not close receiver. Error reading UA\n");
    return -1;
  }
  printf("Sent DISC and Read UA\n");
  printf("Ended connection\n");
  return 0;
}

int llopen(char *port, int caller)
{
  int fileDescriptor = openSerialPort(port, caller);
  if (fileDescriptor < 0)
  {
    return -1;
  }
  if (setNewSettings(fileDescriptor, caller) < 0)
  {
    return -1;
  }

  if (setNewAlarmHandler(alarmHandler))
    return -1;

  if (caller == SENDER)
  {
    if (llopenSender(fileDescriptor) == -1)
    {
      return -1;
    }
  }
  else if (caller == RECEIVER)
  {
    if (llopenReceiver(fileDescriptor) == -1)
    {
      return -1;
    }
  }
  return fileDescriptor;
}

int llopenSender(int fileDescriptor)
{
  unsigned char set_frame[5];
  buildSupervisionFrame(set_frame, SENDER, C_SET, -1);
  if (writeAndReadReply(fileDescriptor, set_frame, 5, C_UA, SENDER) == -1)
  {
    perror("Can't establish connection");
    logToFile("Error in llopenSender");
    return -1;
  }
  printf("Read acknowledge\n");
  printf("Connection opened\n");
  return 0;
}

int llopenReceiver(int fileDescriptor)
{
  unsigned char ua_frame[5];
  buildSupervisionFrame(ua_frame, RECEIVER, C_UA, -1);
  Frame_Header expected_frame;
  expected_frame.address_field = getAddress(SENDER, C_SET);
  expected_frame.control_field = C_SET;
  if (readFrameHeader(fileDescriptor, &expected_frame, 0) != OK)
  {
    perror("Couldn't receive SET");
    logToFile("Error in llopenReceiver");
    return -1;
  }
  printf("Read set frame\n");
  if (write(fileDescriptor, ua_frame, 5) != 5)
  {
    printf("Error sending ua\n");
    return -1;
  }
  printf("Sent ua frame\n");
  printf("Connection opened\n");
  return 0;
}

int llread(int sp_fd, unsigned char **data)
{
  Frame_Header expected_frame_header;
  unsigned long data_size;
  expected_frame_header.control_field = (sequence_number % 2) << 6;
  expected_frame_header.address_field = getAddress(SENDER, expected_frame_header.control_field);
  int res = readInformationFrame(sp_fd, &expected_frame_header, data, &data_size);
  if (res == 0)
  {
    sequence_number++;
    unsigned char rr_frame[5];
    printf("Sending RR\n");
    buildSupervisionFrame(rr_frame, RECEIVER, C_RR, sequence_number);
    if (write(sp_fd, rr_frame, 5) != 5)
    {
      printf("Error sending rr_frame\n");
    }
  }
  else if (res == -1)
  {
    unsigned char rej_frame[5];
    printf("Sending REJ\n");
    free(*data);
    *data = NULL;
    buildSupervisionFrame(rej_frame, RECEIVER, C_REJ, sequence_number);
    if (write(sp_fd, rej_frame, 5) != 5)
    {
      printf("Error sending rej_frame\n");
    }
  }
  return data_size;
}

int llwrite(int sp_fd, unsigned char *data, unsigned long data_size)
{
  unsigned char *frame = NULL;
  unsigned char expected_control_field = C_RR ^ ((sequence_number + 1) % 2) << 7;
  unsigned long frame_size = buildInformationFrame(&frame, data, data_size, sequence_number);
  sequence_number++;
  byteStuffing(&frame, &frame_size);
  int res = writeAndReadReply(sp_fd, frame, frame_size, expected_control_field, SENDER);
  return res;
}

int readDataToArray(int sp_fd, unsigned char **data, unsigned long *data_size)
{
  (*data_size) = 0;
  unsigned long frame_allocated_space = 1;
  (*data) = malloc(frame_allocated_space * sizeof(unsigned char));
  unsigned char read_char;
  while (1)
  {
    if (read(sp_fd, &read_char, 1) < 1)
    {
      continue;
    }
    if (read_char == FLAG)
    {
      break;
    }
    if ((*data_size) >= frame_allocated_space)
    {
      frame_allocated_space *= 2;
      *data = realloc((*data), frame_allocated_space);
      if ((*data) == NULL)
      {
        perror("Error realloc memory for read data frame");
        return -1;
      }
    }
    (*data)[(*data_size)++] = read_char;
  }
  return 0;
}

Reply_Status readFrameHeader(int sp_fd, Frame_Header *expected_frame_header, int isData)
{
  State state = START;
  unsigned char read_char;
  unsigned char received_address;
  unsigned char received_control;
  printf("Expected address_field 0x%02X\n", expected_frame_header->address_field);
  printf("Expected control_field 0x%02X\n", expected_frame_header->control_field);
  int isDuplicated = 0;
  int isReject = 0;
  flag = 0;
  while (state != STOP && !flag)
  {
    printf("State = %d\n", state);
    if (state == BCC1_OK && isData)
    {
      logToFile("readFrameHeader : State - BBC1_OK with data");
      state = STOP;
      break;
    }
    if (read(sp_fd, &read_char, 1) < 1)
    {
      continue;
    }

    printf("0x%02X\n", read_char);
    switch (state)
    {
    case START:
    {
      logToFile("readFrameHeader : State - START");
      if (read_char == FLAG)
      {
        state = FLAG_REC;
      }
      break;
    }
    case FLAG_REC:
    {
      logToFile("readFrameHeader : State - FLAG_REC");
      received_address = read_char;
      if (read_char == expected_frame_header->address_field)
      {
        state = A_REC;
      }
      else if (read_char != FLAG)
      {
        state = START;
      }
      break;
    }
    case A_REC:
    {
      logToFile("readFrameHeader : State - A_REC");
      received_control = read_char;
      if (read_char == expected_frame_header->control_field)
      {
        state = C_REC;
      }
      else if (isData)
      {
        if (read_char == (expected_frame_header->control_field ^ 0x40))
        {
          state = C_REC;
          isDuplicated = 1;
        }
      }
      else if (read_char & C_REJ)
      {
        state = C_REC;
        isReject = 1;
      }
      else if (read_char == (C_RR | (expected_frame_header->control_field ^ 0x80)))
      {
        isDuplicated = 1;
        state = C_REC;
      }
      else if (read_char == FLAG)
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
      logToFile("readFrameHeader : State - C_REC");
      if (read_char == (received_address ^ received_control))
      {
        state = BCC1_OK;
      }
      else if (read_char == FLAG)
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
      logToFile("readFrameHeader : State - BCC1_OK not data");
      if (read_char == FLAG)
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
      return ERROR;
    }
    }
  }
  logToFile("readFrameHeader : End ");

  if (isDuplicated)
  {
    printf("DUPLICATED\n");
    return DUPLICATED;
  }
  if (isReject)
  {
    printf("REJECTED\n");
    return REJECTED;
  }
  if (state != STOP)
  {
    printf("STATE != STOP\n");
    return ERROR;
  }
  printf("OK\n");
  return OK;
}

int readInformationFrame(int sp_fd, Frame_Header *frame_header, unsigned char **data_unstuffed, unsigned long *data_size)
{
  Reply_Status returnValue = readFrameHeader(sp_fd, frame_header, 1);
  unsigned char *data_bcc2 = NULL;
  unsigned long data_bcc2_size;
  readDataToArray(sp_fd, &data_bcc2, &data_bcc2_size);
  if (returnValue == DUPLICATED)
  {
    // If frame duplicated without errors.
    // Should trigger a receiver ready.
    //flushSP(sp_fd);
    (*data_unstuffed) = NULL;
    free(data_bcc2);
    return 0;
  }
  byteUnstuffing(&data_bcc2, &data_bcc2_size);
  (*data_size) = data_bcc2_size - 1;
  unsigned char received_bcc2 = data_bcc2[(data_bcc2_size)-1];
  (*data_unstuffed) = malloc((*data_size) * sizeof(unsigned char));
  memcpy((*data_unstuffed), data_bcc2, (*data_size));
  unsigned char calculated_bcc2 = getBCC((*data_unstuffed), (*data_size));
  unsigned i;
  printf("[Link Layer] Received data:\n");
  for (i = 0; i < (*data_size); i++)
  {
    printf("0x%02X ", (*data_unstuffed)[i]);
  }
  printf("\n");

  if (calculated_bcc2 == received_bcc2)
  {
    return 0;
  }
  else
  {
    printf("Calculated bcc2=0x%02X\n", calculated_bcc2);
    printf("Received_bcc2=0x%02X\n", received_bcc2);
    // Error in BCC2 -> should trigger a C_REJ
    return -1;
  }
}

int writeAndReadReply(int sp_fd, unsigned char *frame_to_write, unsigned long frame_size, unsigned char expected_control_field, int caller)
{
  Frame_Header frame_header_expected;
  frame_header_expected.address_field = getAddress((caller ^ 1), expected_control_field); //negate the caller
  frame_header_expected.control_field = expected_control_field;

  unsigned int currentTries = 0;
  while (currentTries++ < MAX_TRIES)
  {
    int res;
    if ((res = write(sp_fd, frame_to_write, frame_size)) < frame_size)
    {
      printf("\nError writting\n");
    }
    alarm(3);
    Reply_Status return_value = readFrameHeader(sp_fd, &frame_header_expected, 0); // 0 - CONTROL FRAME
    alarm(0);
    if (return_value == OK || return_value == DUPLICATED)
    {
      logToFile("writeAndReadReply : success");
      break;
    }
    if (return_value == REJECTED)
    {
      currentTries = 0;
    }
  }
  if (currentTries > MAX_TRIES)
  {
    printf("Max tries reached\n");
    return -1;
  }
  return 0;
}
