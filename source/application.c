#include "application.h"


// Counter that keeps track of how many packets have been sent through the serial port.
unsigned long writeCounter = 0;

// Flag used with the alarm and counter that keeps track of how much times the alarm has been activated
int flag=1, conta=1;

/**
* Atende Alarme
*/
void atende()
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
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

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(int pos, char value, char* array, int length){
	int i;
	for (i = length - 1; i >= pos ; i--){
		array[i+1] = array[i];
	}
	array[pos] = value;
}

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, -1 if error.
*/
int llopen(int fileDescriptor, int caller){
  printf("llopen called\n");
  if(caller == TRANSMITTER){
  	char set[5];
    buildControlPacket(C_SET, set);
  	if (sendPacketAndWaitAcknowledge(fileDescriptor, set) == -1){
  		perror("Could not establish connection, make sure the systems are connected\n");
  		exit(-1);
  	}
  }else if (caller == RECEIVER){
    char g_ua[CP_LENGTH];
    if (readControlPacket(fileDescriptor,C_SET) != CP_LENGTH){
      perror("Error reading SET packet");
      exit(-1);
    }
    printf("Received SET\n");
    buildControlPacket(C_UA, g_ua);
  	printf("Sending UA\n");
  	if(write(fileDescriptor, g_ua, CP_LENGTH) <= 0){
      perror("Error sending UA");
      exit(-1);
    };
    printf("UA Sent\n");
  }
	return 0;
}

/**
* A method to read from the file descriptor into the buffer (trama).
*/
int llread(int fd, char* trama) {
	char ch;
	int result;

	//Read the whole trama
	printf("Started reading trama\n");
	int index = 0;
	int found_flag = 0;
	while(found_flag < 2) {
		result = read(fd, &ch, 1);
		if(result == 1){
			trama[index++] = ch;
			printf("Char: %x\n", ch);
			if(ch == FLAG)
				found_flag++;
		} else {
			printf("Failed to read\n");
		}
	}
	printf("Finished reading trama\n");

	//Check BCC1
	char address = trama[1];
	char controll = trama[2];
	char bcc1 = trama[3];
	if((address^controll) != bcc1){
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
		return -1;
	}

	return 0;
}

/**
* Writes a buffer array to the fileDescriptor.
* length - Length of the array to send
* Note: The buffer will be processed with byte stuffing before being sent.
*/
int llwrite(int fileDescriptor, char* buffer, int length){
	int i, res;
	char bcc2 = 0;
	for (i = 0; i < length; i++){
    bcc2 = bcc2 ^ buffer[i];
		if (buffer[i] == 0x7e || buffer[i] == 0x7d){
			insertValueAtPos(i,ESCAPE_CHAR,buffer,length);
			length++;
			i++;
			buffer[i] = buffer[i] ^ 0x20;
			printf("Escaped char!\n");
		}
	}
	printf("BCC and byte stuffing complete\n");
  int sizeOfPacket = (length + 6) * sizeof(char);
	char* trama = malloc(sizeOfPacket);
	trama[0] = FLAG;
	trama[1] = A;
	trama[2] = (char)(writeCounter++ % 2);
	trama[3] = trama[1] ^ trama[2];
	memcpy(&trama[4], buffer, length);
	trama[length + 4] = bcc2;
	trama[length + 5] = FLAG;
	res = write(fileDescriptor,trama,sizeof(trama)/sizeof(trama[0]));
	if (res <= 0){
		perror("Error writing to serial port, exiting...");
		exit(-1);
	}
	printf("Information sent: %d bytes written\n", res);
	// Get Receiver READY
	printf("Looking for receiver ready");
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
  char read_char;
  char package_received[5];
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
* Sends a given packet and waits for unumberd acknowledge
*/
int sendPacketAndWaitAcknowledge(int fileDescriptor, char* packet){
  int res; //Used to store the return of write() and read() calls.
  int state;
  conta = 1;
  while(conta < 4){
    if(flag){
      alarm(3);
      //Try to send message
      res = write(fileDescriptor, packet, 5);
      printf("Try number %d, %d bytes written\n", conta, res);
      state = readControlPacket(fileDescriptor,C_UA);
      if (state == 5){
        printf("Acknowledge received\n");
        return 0;
      }
    }
  }
  return -1;
}
