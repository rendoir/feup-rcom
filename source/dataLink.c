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
  printf("Alarm #%d\n", alarm_tries);
  flag = 1;
  alarm_tries++;
}

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(size_t pos, char value, char ** array, int length)
{
  size_t i;
  for (i = length - 1; i >= pos; i--)
  {
    (*array)[i + 1] = (*array)[i];
  }
  (*array)[pos] = value;
}

/**
 * Makes the Frame for sender/receiver
 */
void buildFrame(char **frame, char **buffer, unsigned length, unsigned char bcc2)
{
  printf("<BUILD_FRAME>\n");
  (*frame)[0] = FLAG;
  (*frame)[1] = A;
  (*frame)[2] = (char)(writeCounter++ % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  printf("    Copying buffer to frame\n");
  memcpy(&(*frame)[4], buffer, length);
  printf("    Memory copied\n");
  (*frame)[length + 4] = bcc2;
  (*frame)[length + 5] = FLAG;
  printf("</BUILD_FRAME>\n");
}

/*------------------------------------*/
/*------------------------------------*/
/*-----------CONTROL PACKET-----------*/
/*------------------------------------*/
/*------------------------------------*/

/**
* Makes a control/supervision frame with the given control byte/field.
*/
void buildControlFrame(char controlByte, char **frame)
{
  printf("<BUILD_CONTROL_FRAME>\n");
  (*frame) = malloc(CP_LENGTH * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A;
  (*frame)[2] = controlByte;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  (*frame)[4] = FLAG;
  printf("</BUILD_CONTROL_FRAME>\n");
}

/**
* Reads a control frame from the file descriptor
* Returns: -1 if error detected, -2 if alarm fired, the control byte read otherwise.
*/
char readControlFrame(int fileDescriptor)
{
  //Read from serial port

  int res;
  unsigned char read_char;
  unsigned char frame_received[5];
  int state = 0;
  flag = 0;
  printf("Reading Control Frame\n");
  while (state != 5 && flag == 0)
  {
    res = read(fileDescriptor, &read_char, 1);
    if (res > 0)
    {
      printf("current state = %d\n", state);
      printf("read byte: 0x%x\n", read_char);
      frame_received[state] = read_char;
      switch(state){
        case 0:{
          if (read_char == FLAG){
            state = 1;
          }
          break;
        }
        case 1:{
          if (read_char != FLAG){
            state = 2;
          }else{
            state = 1;
          }
          break;
        }
        case 2:{
          if ( read_char != FLAG) {
            state = 3;
          }else{
            state = 1;
          }
          break;
        }
        case 3:{
          if ( (frame_received[1] ^ frame_received[2]) == read_char){
            state = 4;
          }else{
            printf("BCC error\n");
            return -1;
          }
          break;
        }
        case 4:{
          if (read_char == FLAG){
            state = 5;
          }else{
            state = 0;
          }
          break;
        }
      }
    }
  }

  if (state == 5)
  {
    return frame_received[2]; // control byte
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
    return llopenTransmitter(fileDescriptor);
  }
  else if (caller == RECEIVER)
  {
    return llopenReceiver(fileDescriptor);
  }
  return -1;
}

/**
* byte stuffing assumed for I frames
*/
int sendImportantFrame(int fd, char* frame, int length){
  flag=1;
  alarm_tries = 1;
  int res;
  while(alarm_tries < MAX_TRIES){
    if (flag){
      alarm(TIME_OUT);
      flag=0;
      printf("Writing to serial port fd=%d\n",fd);
      res = write(fd,frame,length);
      if (res > 0){
        printf("Written %d bytes to serial port\n",res);
        char controlField = readControlFrame(fd);
        if (controlField == C_UA){
          printf("UA received\n");
          alarm(0);
          break;
        }else if ((controlField & C_RR) == C_RR){
          printf("RR received\n");
          alarm(0);
          break;
        }else if ((controlField & C_REJ) == C_REJ){
          printf("REJ received\n");
          alarm(0);
          alarm_tries = 1;
          flag = 1;
        }else if (controlField == -1){ //bcc error
          printf("BCC error\n");
          alarm(0);
          alarm_tries = 1;
          flag = 1;
        }else{
          printf("TIMEOUT\n");
        }
      }
    }
  }
  if (alarm_tries >= MAX_TRIES){
    return -1;
  }
  return 0;
}

/**
* Opens/establish the connection.
* when caller is TRANSMITTER
* Return: fd if success, negative on error.
*/
int llopenTransmitter(int fileDescriptor)
{
  int res;
  printf("\n<LLOPEN>\n");
  char *set = NULL;
  buildControlFrame(C_SET, &set);
  res = sendImportantFrame(fileDescriptor,set,5);
  if (res < 0){
    perror("Could not establish connection");
    return -1;
  }
  printf("\n<LLOPEN/>\n");
  return fileDescriptor;
}

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: fd if success, negative on error.
*/
int llopenReceiver(int fileDescriptor)
{
  char *g_ua = NULL;
  char controlByte = readControlFrame(fileDescriptor);
  if (controlByte < 0)
  {
    printf("Error reading SET frame, error no: %d", controlByte);
    return controlByte;
  }
  if (controlByte == C_SET){
    printf("    Received SET\n");
    buildControlFrame(C_UA, &g_ua);
    printf("    Sending UA\n");
    if (write(fileDescriptor, g_ua, CP_LENGTH) != CP_LENGTH)
    {
      perror("Error sending UA");
      return -1;
    }
    printf("    UA Sent\n");
  }
  printf("\n<LLOPEN/>\n");
  return fileDescriptor;
}

/*------------------------------------*/
/*------------------------------------*/
/*-------------LLWRITE----------------*/
/*------------------------------------*/
/*------------------------------------*/

int stuffingBuffer(char** buffer, unsigned *size, char *bcc2)
{
  size_t i;
  size_t allocatedSpace = *size;
  for (i = 0; i < *size; i++)
  {
    *bcc2 = *bcc2 ^ (*buffer)[i];
    if ((*buffer)[i] == 0x7e || (*buffer)[i] == 0x7d)
    {
      if (*size >= allocatedSpace)
      {
        allocatedSpace = *size * 2;
        (*buffer) = realloc((*buffer),allocatedSpace);
      }
      insertValueAtPos(i, ESCAPE_CHAR, buffer, *size);
      (*size)++;
      i++;
      (*buffer)[i] = (*buffer)[i] ^ 0x20;
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
  int res;
  //do stuffing of buffer

  stuffingBuffer(&buffer, &size, &bcc2);

  printf("    BCC and byte stuffing complete\n");
  int sizeOfFrame = (size + 6) * sizeof(char);
  char *frame = malloc(sizeOfFrame);

  buildFrame(&frame, &buffer, size, bcc2);

  res = sendImportantFrame(fileDescriptor,frame,sizeOfFrame);
  if (res < 0){
    perror("Check your connection");
    return -1;
  }

  printf("<LLWRITE/>\n");
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
  char* receiver_ready = NULL;
  control = control << 1;
  control = control ^ 0x80;
  unsigned char recReadyByte = C_RR | control;
  buildControlFrame(recReadyByte, &receiver_ready);
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

  char* disc_frame = NULL;
  buildControlFrame(C_DISC, &disc_frame);
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
  char controlByte = sendImportantFrame(*fileDescriptor, disc_frame, CP_LENGTH);
  if (controlByte < 0)
  {
    perror("Could not establish connection, make sure the systems are connected");
    return -1;
  }else if(controlByte != C_DISC){
    printf("Expecting disconnect packet but received 0x%02x\n",controlByte);
    return -1;
  }else{
    printf("    DISC Sent and DISC Received, Sending UA");

    char* ua_frame = NULL;
    buildControlFrame(C_UA, &ua_frame);

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
