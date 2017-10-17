#include "dataLink.h"

// Counter that keeps track of how many packets have been sent through the serial port.
unsigned long writeCounter = 0;

// Flag used with the alarm and counter that keeps track of how much times the alarm has been activated
int flag = 1, alarm_tries = 1;

/**
* Handles alarm
*/
void alarm_handler()
{
  printf("    Alarm #%d\n", alarm_tries);
  flag = 1;
  alarm_tries++;
}

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(size_t pos, char value, char *array, int length)
{
  size_t i;
  for (i = length - 1; i >= pos; i--)
  {
    array[i + 1] = array[i];
  }
  array[pos] = value;
}

/**
 * Makes the Trama for sender/receiver
 */
void buildTrama(char *trama, char *buffer, unsigned length, unsigned char bcc2)
{
  trama[0] = FLAG;
  trama[1] = A;
  trama[2] = (char)(writeCounter++ % 2) << 6;
  trama[3] = trama[1] ^ trama[2];
  memcpy(&trama[4], buffer, length);
  trama[length + 4] = bcc2;
  trama[length + 5] = FLAG;
}

/*------------------------------------*/
/*------------------------------------*/
/*-----------CONTROL PACKET-----------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* Makes a control/supervision packet with the given control byte/field.
*/
void buildControlPacket(char controlByte, char *packet)
{
  packet[0] = FLAG;
  packet[1] = A;
  packet[2] = controlByte;
  packet[3] = packet[1] ^ packet[2];
  packet[4] = FLAG;
}

/**
* Reads a control packet from the file descriptor and returns the current state.
*/
int readControlPacket(int fileDescriptor, char expectedControlByte)
{
  //Read from serial port
  flag = 0; // To enable the while loop to be performed
  int res;
  unsigned char read_char;

  State_Machine sm;
  unsigned char expected_flag[4];
  expected_flag[0] = FLAG;
  expected_flag[1] = A;
  expected_flag[2] = expectedControlByte;
  expected_flag[4] = FLAG;
  init_State_Machine(&sm, expected_flag);
  printf("Reading Control Packet\n");

  while (!endOfStateMachine(&sm) && flag == 0)
  {
    res = read(fileDescriptor, &read_char, 1);
    if (res > 0)
    {
      printf("current state = %d\n", sm.state_id);
      printf("read byte: 0x%x\n", read_char);

      //State Machine
      next_State(&sm, &read_char);
    }
  }

  if (endOfStateMachine(&sm))
  {
    printf("Acknowledge received\n");
    return 0;
  }

  return -1;
}

/**
* Sends a given packet and waits for a given response.
* <param responseByte> Char that is expected to be read in the control packet </param>
*/
int sendPacketAndWaitResponse(int fileDescriptor, char *packet, char responseByte)
{
  int res; //Used to store the return of write() and read() calls.
  alarm_tries = 1;
  while (alarm_tries < MAX_TRIES)
  {
    if (flag)
    {
      alarm(TIME_OUT);
      //Try to send message
      res = write(fileDescriptor, packet, 5);
      printf("Try number %d, %d bytes written\n", alarm_tries, res);
      if (!readControlPacket(fileDescriptor, responseByte))
        return 0;
    }
  }
  return -1;
}

/*------------------------------------*/
/*------------------------------------*/
/*--------------LLOPEN----------------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, -1 if error.
*/
int llopen(int port, int caller)
{
  printf("\n<LLOPEN>\n");
  int fileDescriptor;
  fileDescriptor = openSerialPort(port, caller);
  serialPort_setNewSettigns(fileDescriptor, caller);

  if (fileDescriptor < 0)
    return -1;
  if (caller == TRANSMITTER)
  {
    return llopenTrasnmitter(fileDescriptor);
  }
  else if (caller == RECEIVER)
  {
    return llopenReceiver(fileDescriptor);
  }
  return -1;
}

/**
* Opens/establish the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llopenTrasnmitter(int fileDescriptor)
{
  char set[5];
  buildControlPacket(C_SET, set);
  if (sendPacketAndWaitResponse(fileDescriptor, set, C_UA) < 0)
  {
    perror("Could not establish connection, make sure the systems are connected\n");
    return -2;
  }
  printf("    SET Sent and UA Received");
  printf("\n<LLOPEN/>\n");
  return 0;
}

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llopenReceiver(int fileDescriptor)
{
  char g_ua[CP_LENGTH];
  if (readControlPacket(fileDescriptor, C_SET) != CP_LENGTH)
  {
    perror("Error reading SET packet");
    return -3;
  }
  printf("    Received SET\n");
  buildControlPacket(C_UA, g_ua);
  printf("    Sending UA\n");
  if (write(fileDescriptor, g_ua, CP_LENGTH) != CP_LENGTH)
  {
    perror("Error sending UA");
    return -4;
  }
  printf("    UA Sent\n");
  printf("\n<LLOPEN/>\n");
  return 0;
}

/*------------------------------------*/
/*------------------------------------*/
/*-------------LLWRITE----------------*/
/*------------------------------------*/
/*------------------------------------*/

int stuffingBuffer(String *bufferString, unsigned *size, char *bcc2)
{
  size_t i;
  char *buffer = (*bufferString).buffer;
  for (i = 0; i < *size; i++)
  {
    *bcc2 = *bcc2 ^ buffer[i];
    if (buffer[i] == 0x7e || buffer[i] == 0x7d)
    {
      if (*size >= (*bufferString).allocatedSpace)
      {
        doubleMemoryAllocated(bufferString);
      }
      insertValueAtPos(i, ESCAPE_CHAR, buffer, *size);
      (*size)++;
      i++;
      buffer[i] = buffer[i] ^ 0x20;
      printf("    Escaped char\n");
    }
  }
  return 0;
}

/**
* Writes a buffer array to the fileDescriptor.
* length - Length of the array to send
* Note: The buffer will be processed with byte stuffing before being sent.
*/
int llwrite(int fileDescriptor, char *buffer, unsigned size)
{
  printf("<LLWRITE>\n");
  char bcc2 = 0;
  String *bufferString;
  init_string(bufferString, buffer, size);

  //do stuffing of buffer
  stuffingBuffer(bufferString, &size, &bcc2);

  printf("    BCC and byte stuffing complete\n");
  int sizeOfPacket = (size + 6) * sizeof(char);
  char *trama = malloc(sizeOfPacket);

  buildTrama(trama, buffer, size, bcc2);
  destroy(bufferString);

  int res;
  res = write(fileDescriptor, trama, sizeOfPacket);
  if (res <= 0)
  {
    perror("Error writing to serial port");
    return -1;
  }
  printf("    Information sent: %d bytes written\n", res);

  // Get Receiver Ready
  printf("    Looking for receiver ready");
  char control = trama[2] << 1;
  control = control ^ 0x80;
  char recReadyByte = C_RR | control;
  if (readControlPacket(fileDescriptor, recReadyByte) < 0)
  {
    printf("    Receiver ready not received");
    return -1;
  }

  printf("    Receiver ready received\n <LLWRITE/>\n");
  return 0;
}

/*------------------------------------*/
/*------------------------------------*/
/*--------------LLREAD----------------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* A method to read from the file descriptor into the buffer (trama).
*/
int llread(int fd, char *trama)
{
  printf("<LLREAD>\n");

  char ch;
  int result;

  //Read the whole trama (flag to flag)
  printf("    Started reading trama\n");
  int index = 0;
  int found_flag = 0;
  while (found_flag < 2)
  {
    result = read(fd, &ch, 1);
    if (result == 1)
    {
      trama[index++] = ch;
      printf("    Char: %02x\n", ch);
      if (ch == FLAG)
        found_flag++;
    }
    else
      printf("    Failed to read\n");
  }
  printf("    Finished reading trama\n");

  //Check BCC1
  char address = trama[1];
  char control = trama[2];
  char bcc1 = trama[3];
  if ((address ^ control) != bcc1)
  {
    perror("BCC1 Incorrect parity");
    return -1;
  }

  //Check BCC2 and unstuff
  int size = index;
  char expected_bcc2 = trama[size - 2];
  char calculated_bcc2 = 0;
  int i;
  for (i = 4; i < size - 2; i++)
  {
    calculated_bcc2 ^= trama[i];
  }
  if (expected_bcc2 != calculated_bcc2)
  {
    perror("BCC2 Incorrect parity");
    return -2;
  }

  //Send receiver ready
  char receiver_ready[5];
  control = control << 1;
  control = control ^ 0x80;
  unsigned char recReadyByte = C_RR | control;
  buildControlPacket(recReadyByte, receiver_ready);
  printf("    Sending RR = 0x%02x\n", recReadyByte);
  if (write(fd, receiver_ready, CP_LENGTH) <= 0)
  {
    perror("Error sending RR");
    return -1;
  }
  printf("    RR Sent\n");

  printf("<LLREAD/>\n");
  return 0;
}

/*------------------------------------*/
/*------------------------------------*/
/*-------------LLCLOSE----------------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* Close the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llclose(int fileDescriptor, int caller)
{
  printf("\n<LLCLOSE>\n");
  if (fileDescriptor < 0)
  {
    return -1;
  }

  char disc_packet[5];
  buildControlPacket(C_DISC, disc_packet);
  if (caller == TRANSMITTER)
  {
    return llcloseTransmitter(&fileDescriptor, disc_packet);
  }
  else if (caller == RECEIVER)
  {
    return llcloseReceiver(&fileDescriptor, disc_packet);
  }
  return -1;
}

/**
* Close the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llcloseTransmitter(const int *fileDescriptor, char *disc_packet)
{
  if (sendPacketAndWaitResponse(*fileDescriptor, disc_packet, C_DISC) < 0)
  {
    perror("Could not establish connection, make sure the systems are connected\n");
    return -2;
  }
  printf("    DISC Sent and DISC Received, Sending UA");

  char ua_packet[5];
  buildControlPacket(C_UA, ua_packet);

  if (write(*fileDescriptor, ua_packet, CP_LENGTH) < 0)
  {
    perror("    Error sending UA");
  }

  printf("\n<LLCLOSE/>\n");
  return 0;
}

/**
* Close the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llcloseReceiver(const int *fileDescriptor, char *disc_packet)
{

  if (readControlPacket(*fileDescriptor, C_DISC) != CP_LENGTH)
  {
    perror("Error reading DISC packet");
    return -3;
  }

  if (write(*fileDescriptor, disc_packet, CP_LENGTH) < 0)
  {
    perror("    Error sending disc packet");
  }

  closeSerialPort_and_SetOldSettigns(fileDescriptor);

  printf("\n<LLCLOSE/>\n");
  return 0;
}
