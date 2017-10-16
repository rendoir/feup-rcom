#include "dataLink.h"


// Counter that keeps track of how many packets have been sent through the serial port.
unsigned long writeCounter = 0;

// Flag used with the alarm and counter that keeps track of how much times the alarm has been activated
int flag=1, alarm_tries=1;

/**
* Handles alarm
*/
void alarm_handler()
{
  printf("    Alarm #%d\n", alarm_tries);
  flag=1;
  alarm_tries++;
}

/**
* Makes a control/supervision packet with the given control byte/field.
*/
void buildControlPacket(char controlByte, char* packet){
  packet[0] = FLAG;
  packet[1] = A;
  packet[2] = controlByte;
  packet[3] = packet[1] ^ packet[2];
  packet[4] = FLAG;
}

void buildTrama(char* trama, char* buffer, unsigned length, unsigned char bcc2){
  trama[0] = FLAG;
  trama[1] = A;
  trama[2] = (char)(writeCounter++ % 2) << 6;
  trama[3] = trama[1] ^ trama[2];
  memcpy(&trama[4], buffer, length);
  trama[length + 4] = bcc2;
  trama[length + 5] = FLAG;
}

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(int pos, char value, char* array, int length){
  int i;
  for (i = length - 1; i >= pos; i--){
    array[i+1] = array[i];
  }
  array[pos] = value;
}

/*
* Closes the connection sending a DISCONNECT packet.
*/
int llclose(int fileDescriptor, int caller){
  printf("\n<LLCLOSE>\n");
  if (fileDescriptor < 0){
    return -1;
  }
  char disc_packet[5];
  buildControlPacket(C_DISC,disc_packet);
  if (caller == TRANSMITTER){
    if (sendPacketAndWaitResponse(fileDescriptor, disc_packet,C_DISC) < 0){
      perror("Could not establish connection, make sure the systems are connected\n");
      return -2;
    }
    printf("    DISC Sent and DISC Received, Sending UA");
    char ua_packet[5];
    buildControlPacket(C_UA,ua_packet);
    if (write(fileDescriptor,ua_packet,CP_LENGTH) < 0){
      perror("    Error sending UA");
    }
  } else if (caller == RECEIVER){
      if (readControlPacket(fileDescriptor, C_DISC) != CP_LENGTH){
        perror("Error reading DISC packet");
        return -3;
      }
      if (write(fileDescriptor,disc_packet,CP_LENGTH) < 0){
        perror("    Error sending disc packet");
      }
  }
  printf("\n<LLCLOSE/>\n");
  return 0;
}

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llopen(int fileDescriptor, int caller){
  printf("\n<LLOPEN>\n");

  if(fileDescriptor < 0)
    return -1;
  if(caller == TRANSMITTER){
    char set[5];
    buildControlPacket(C_SET, set);
    if (sendPacketAndWaitResponse(fileDescriptor, set,C_UA) < 0){
      perror("Could not establish connection, make sure the systems are connected\n");
      return -2;
    }
    printf("    SET Sent and UA Received");
  } else if (caller == RECEIVER){
    char g_ua[CP_LENGTH];
    if (readControlPacket(fileDescriptor, C_SET) != CP_LENGTH){
      perror("Error reading SET packet");
      return -3;
    }
    printf("    Received SET\n");
    buildControlPacket(C_UA, g_ua);
    printf("    Sending UA\n");
    if(write(fileDescriptor, g_ua, CP_LENGTH) != CP_LENGTH){
      perror("Error sending UA");
      return -4;
    }
    printf("    UA Sent\n");
  }
  printf("<LLOPEN/>\n");
  return 0;
}

/**
* A method to read from the file descriptor into the buffer (trama).
*/
int llread(int fd, char* trama) {
  printf("<LLREAD>\n");

  char ch;
  int result;

  //Read the whole trama (flag to flag)
  printf("    Started reading trama\n");
  int index = 0;
  int found_flag = 0;
  while(found_flag < 2) {
    result = read(fd, &ch, 1);
    if(result == 1){
      trama[index++] = ch;
      printf("    Char: %02x\n", ch);
      if(ch == FLAG)
        found_flag++;
      } else
          printf("    Failed to read\n");
  }
  printf("    Finished reading trama\n");

  //Check BCC1
  char address = trama[1];
  char control = trama[2];
  char bcc1 = trama[3];
  if((address^control) != bcc1){
    perror("BCC1 Incorrect parity");
    return -1;
  }

  //Check BCC2 and unstuff
  int size = index;
  char expected_bcc2 = trama[size - 2];
  char calculated_bcc2 = 0;
  int i;
  for(i = 4; i < size - 2; i++) {
    calculated_bcc2 ^= trama[i];
  }
  if(expected_bcc2 != calculated_bcc2){
    perror("BCC2 Incorrect parity");
    return -2;
  }

  //Send receiver ready
  char receiver_ready[5];
  control = control << 1;
  control = control ^ 0x80;
  unsigned char recReadyByte = C_RR | control;
  buildControlPacket(recReadyByte,receiver_ready);
  printf("    Sending RR = 0x%02x\n", recReadyByte);
  if(write(fd, receiver_ready, CP_LENGTH) <= 0){
    perror("Error sending RR");
    exit(-1);
  }
  printf("    RR Sent\n");

  printf("<LLREAD/>\n");
  return 0;
}

/**
* Writes a buffer array to the fileDescriptor.
* length - Length of the array to send
* Note: The buffer will be processed with byte stuffing before being sent.
*/
int llwrite(int fileDescriptor, char* buffer, unsigned size){
  printf("<LLWRITE>\n");
  int i, res;
  char bcc2 = 0;
  for (i = 0; i < size; i++){
    bcc2 = bcc2 ^ buffer[i];
    if (buffer[i] == 0x7e || buffer[i] == 0x7d){
      insertValueAtPos(i, ESCAPE_CHAR, buffer, size);
      size++;
      i++;
      buffer[i] = buffer[i] ^ 0x20;
      printf("    Escaped char\n");
    }
  }
  printf("    BCC and byte stuffing complete\n");
  int sizeOfPacket = (size + 6) * sizeof(char);
  char* trama = malloc(sizeOfPacket);
  buildTrama(trama, buffer, size, bcc2);
  res = write(fileDescriptor, trama, sizeOfPacket);
  if (res <= 0){
    perror("Error writing to serial port");
    return -1;
  }
  printf("    Information sent: %d bytes written\n", res);
  // Get Receiver Ready
  printf("    Looking for receiver ready");
  char control = trama[2] << 1;
  control = control ^ 0x80;
  char recReadyByte = C_RR | control;
  if(readControlPacket(fileDescriptor, recReadyByte) < 0){
    printf("    Receiver ready not received");
    return -1;
  } else
    printf("    Receiver ready received");

  printf("<LLWRITE/>\n");
  return 0;
}

/**
* Reads a control packet from the file descriptor and returns the current state.
*/
int readControlPacket(int fileDescriptor, char expectedControlByte){
  //Read from serial port
  int state = 0;
  flag = 0; // To enable the while loop to be performed
  int res;
  unsigned char read_char;
  unsigned char package_received[5];
  printf("Reading Control Packet\n");
  while(state != 5 && flag == 0) {
   res = read(fileDescriptor, &read_char, 1);
   if (res > 0){
     printf("current state = %d\n",state);
     printf("read byte: 0x%x\n", read_char);
     switch(state){
       case 0: {
         if (read_char == FLAG){
           package_received[0] = read_char;
           state = 1;
         }
         break;
       }
       case 1: {
         if (read_char == A) {
           package_received[1] = read_char;
           state = 2;
         } else if (read_char != FLAG) {
           state = 0;
         }
         break;
       }
       case 2: {
         if (read_char == expectedControlByte) {
           package_received[2] = read_char;
           state = 3;
         } else if (read_char == FLAG) {
           state = 1;
         } else {
           state = 0;
         }
         break;
       }
       case 3: {
         if (read_char == (package_received[1]^package_received[2])) {
           state = 4;
         } else {
           state = 0;
         }
         break;
       }
       case 4: {
         if (read_char == FLAG) {
           state = 5;
         } else {
           state = 0;
         }
         break;
       }
     }
   }
  }
  return state;
}


/**
* Sends a given packet and waits for a given response.
*/
int sendPacketAndWaitResponse(int fileDescriptor, char* packet, char responseByte){
  int res; //Used to store the return of write() and read() calls.
  int state;
  alarm_tries = 1;
  while(alarm_tries < MAX_TRIES){
    if(flag){
      alarm(TIME_OUT);
      //Try to send message
      res = write(fileDescriptor, packet, 5);
      printf("Try number %d, %d bytes written\n", alarm_tries, res);
      state = readControlPacket(fileDescriptor,responseByte);
      if (state == 5){
        printf("Acknowledge received\n");
        return 0;
      }
    }
  }
  return -1;
}
