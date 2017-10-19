#include "dataLink.h"

// Counter that keeps track of how many frames have been sent through the serial port.
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
 * Makes the Frame for sender/receiver
 */
void buildFrame(char *frame, char *buffer, unsigned length, unsigned char bcc2)
{
  frame[0] = FLAG;
  frame[1] = A;
  frame[2] = (char)(writeCounter++ % 2) << 6;
  frame[3] = frame[1] ^ frame[2];
  memcpy(&frame[4], buffer, length);
  frame[length + 4] = bcc2;
  frame[length + 5] = FLAG;
}

/*------------------------------------*/
/*------------------------------------*/
/*-----------CONTROL PACKET-----------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* Makes a control/supervision frame with the given control byte/field.
*/
void buildControlFrame(char controlByte, char *frame)
{
  frame[0] = FLAG;
  frame[1] = A;
  frame[2] = controlByte;
  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG;
}

/**
* Reads a control frame from the file descriptor
* Returns: -1 if error detected, -2 if alarm fired, the control byte read otherwise.
*/
char readControlFrame(int fileDescriptor)
{
  //Read from serial port
  flag = 0; // To enable the while loop to be performed
  int res;
  unsigned char read_char;
  unsigned char expected_flag[5];
  unsigned char frame_received[5];
  expected_flag[0] = FLAG;
  expected_flag[1] = A;
  //expected_flag[2] = expectedControlByte;
  expected_flag[4] = FLAG;
  int state = 0;
  printf("Reading Control Frame\n");

  while (state != 5 && flag == 0)
  {
    res = read(fileDescriptor, &read_char, 1);
    if (res > 0)
    {
      printf("current state = %d\n", state);
      printf("read byte: 0x%x\n", read_char);
      frame_received[state] = res;
      if (state == 2){
        continue;
      }
      if (state == 3){
        expected_flag[3] = frame_received[1] ^ frame_received[2];
      }
      if (frame_received[state] == expected_flag[state]){
        state++;
      }
    }
  }

  if (state == 5)
  {
    return frame_received[2]; // control byte
  }

  return -2;
}

/**
* Sends a given frame and waits for a given response.
* <param responseByte> Char that is expected to be read in the control frame </param>
*/
char sendControlFrameAndWait(int fileDescriptor, char *frame)
{
  int res; //Used to store the return of write() and read() calls.
  alarm_tries = 1;
  unsigned char controlByte;
  while (alarm_tries < MAX_TRIES)
  {
    if (flag)
    {
      alarm(TIME_OUT);
      //Try to send message
      res = write(fileDescriptor, frame, 5);
      printf("Try number %d, %d bytes written\n", alarm_tries, res);
      controlByte = readControlFrame(fileDescriptor);
      printf("sendControlFrameAndWait -> controlByte = 0x%02x\n", controlByte);
      alarm(0); // cancel previous alarm
      if (controlByte != -2){ // do not return if readControlFrame returned cause of alarm.
        return controlByte;
      }
    }
  }
  return -2;
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
  buildControlFrame(C_SET, set);
  int max_tries = 3;
  int current_try = 0;
  char controlByte;
  while ((controlByte = sendControlFrameAndWait(fileDescriptor, set)) < 0 && (current_try < max_tries))
  {
    if (controlByte == -2){
      perror("Response not received, check your connection\n");
      return -2;
    }
    current_try++;
  }
  if (current_try < max_tries){
    if (controlByte == C_UA){
      printf("    SET Sent and UA Received");
    }
  }else{
    perror("Response is being received but state machine keeps throwing error!");
  }
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
  char controlByte = readControlFrame(fileDescriptor);
  if (controlByte < 0)
  {
    printf("Error reading SET frame, error no: %d", controlByte);
    return controlByte;
  }
  if (controlByte == C_SET){
    printf("    Received SET\n");
    buildControlFrame(C_UA, g_ua);
    printf("    Sending UA\n");
    if (write(fileDescriptor, g_ua, CP_LENGTH) != CP_LENGTH)
    {
      perror("Error sending UA");
      return -1;
    }
    printf("    UA Sent\n");
  }
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

char sendInfoFrameAndWait(int fileDescriptor, char *frame, int sizeOfFrame){
  int res; //Used to store the return of write() and read() calls.
  alarm_tries = 1;
  char control = frame[2] << 1;
  control = control ^ 0x80;
  char recReadyByte = C_RR | control;
  char recRejectByte = C_REJ | control;
  unsigned char controlByte;
  while (alarm_tries < MAX_TRIES)
  {
    if (flag)
    {
      alarm(TIME_OUT);
      //Try to send message
      res = write(fileDescriptor, frame, sizeOfFrame);
      printf("Try number %d, %d bytes written\n", alarm_tries, res);
      controlByte = readControlFrame(fileDescriptor);
      alarm(0); // cancel previous alarm
      if (controlByte == recReadyByte){
        printf("    Receiver Ready\n");
        return 0;
      }else if (controlByte == recRejectByte){
        printf("    Receiever Rejected, Trying again\n");
        alarm_tries = 1;
        flag = 1;
      }else {
        printf("    Received unexpected byte: 0x%02x",controlByte);
      }
    }
  }
  return -2;
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
  String *bufferString = NULL;
  init_string(bufferString, buffer, size);

  //do stuffing of buffer
  stuffingBuffer(bufferString, &size, &bcc2);

  printf("    BCC and byte stuffing complete\n");
  int sizeOfFrame = (size + 6) * sizeof(char);
  char *frame = malloc(sizeOfFrame);

  buildFrame(frame, buffer, size, bcc2);
  destroy(bufferString);

  if (sendInfoFrameAndWait(fileDescriptor,frame,sizeOfFrame) == -2){
    printf("MAX_TRIES achieved\n");
    return -2;
  }else{
    printf("<LLWRITE/>\n");
  }
  return 0;
}

/*------------------------------------*/
/*------------------------------------*/
/*--------------LLREAD----------------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* A method to read from the file descriptor into the buffer (frame).
*/
int llread(int fd, char *frame)
{
  printf("<LLREAD>\n");

  char ch;
  int result;

  //Read the whole frame (flag to flag)
  printf("    Started reading frame\n");
  int index = 0;
  int found_flag = 0;
  while (found_flag < 2)
  {
    result = read(fd, &ch, 1);
    if (result == 1)
    {
      frame[index++] = ch;
      printf("    Char: %02x\n", ch);
      if (ch == FLAG)
        found_flag++;
    }
    else
      printf("    Failed to read\n");
  }
  printf("    Finished reading frame\n");

  //Check BCC1
  char address = frame[1];
  char control = frame[2];
  char bcc1 = frame[3];
  if ((address ^ control) != bcc1)
  {
    perror("BCC1 Incorrect parity");
    return -1;
  }

  //Check BCC2 and unstuff
  int size = index;
  char expected_bcc2 = frame[size - 2];
  char calculated_bcc2 = 0;
  int i;
  for (i = 4; i < size - 2; i++)
  {
    calculated_bcc2 ^= frame[i];
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
  buildControlFrame(recReadyByte, receiver_ready);
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

  char disc_frame[5];
  buildControlFrame(C_DISC, disc_frame);
  if (caller == TRANSMITTER)
  {
    return llcloseTransmitter(&fileDescriptor, disc_frame);
  }
  else if (caller == RECEIVER)
  {
    return llcloseReceiver(&fileDescriptor, disc_frame);
  }
  return -1;
}

/**
* Close the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llcloseTransmitter(const int *fileDescriptor, char *disc_frame)
{
  char controlByte = sendControlFrameAndWait(*fileDescriptor, disc_frame);
  if (controlByte < 0)
  {
    if (controlByte == -1){
      perror("Error in state machine");
    }else{
      perror("Could not establish connection, make sure the systems are connected");
    }
    return controlByte;
  }else if(controlByte != C_DISC){
    printf("Expecting disconnect packet but received 0x%02x\n",controlByte);
    return -1;
  }else{
    printf("    DISC Sent and DISC Received, Sending UA");

    char ua_frame[5];
    buildControlFrame(C_UA, ua_frame);

    if (write(*fileDescriptor, ua_frame, CP_LENGTH) < 0)
    {
      perror("    Error sending UA");
    }

    printf("\n<LLCLOSE/>\n");
  }
  return 0;
}

/**
* Close the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llcloseReceiver(const int *fileDescriptor, char *disc_frame)
{
  char controlByte = readControlFrame(*fileDescriptor);
  if (controlByte != C_DISC)
  {
    perror("Error reading DISC frame");
    return -1;
  }

  if (write(*fileDescriptor, disc_frame, CP_LENGTH) < 0)
  {
    perror("    Error sending disc frame");
  }

  closeSerialPort_and_SetOldSettigns(fileDescriptor);

  printf("\n<LLCLOSE/>\n");
  return 0;
}
