#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FLAG   0x7e // delimiter flag
#define A      0X03 // address

#define C_SET  0x03 // set up
#define C_DISC 0x0B // disconnect
#define C_UA   0x07 // unnumbered acknowledgment
#define C_RR	 0x05 // receiver ready / positive acknowledge
#define C_REJ	 0x01 // reject / negative acknowledge

#define TRANSMITTER 0
#define RECEIVER		1

#define CP_LENGTH 5 // control packet length

#define ESCAPE_CHAR 0x7d

#define TIME_OUT 3
#define MAX_TRIES 4


/**
* Atende Alarme
*/
void alarm_handler();

/**
* Makes a control/supervision packet with the given control byte/field.
*/
void buildControlPacket(char controlByte, char* packet);

/**
 * Makes the Trama for sender/receiver
 */
void buildTrama(char* trama, char* buffer, unsigned length, unsigned char bcc2);

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(int pos, char value, char* array, int length);

/*
* Closes the connection sending a DISCONNECT packet.
*/
int llclose(int fileDescriptor, int caller);

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, -1 if error.
*/
int llopen(int fileDescriptor, int caller);

/**
* A method to read from the file descriptor into the buffer (trama).
*/
int llread(int fd, char* trama);

/**
* Writes a buffer array to the fileDescriptor.
* length - Length of the array to send
* Note: The buffer will be processed with byte stuffing before being sent.
*/
int llwrite(int fileDescriptor, char* buffer, unsigned length);

/**
* Reads a control packet from the file descriptor and returns the current state.
*/
int readControlPacket(int fileDescriptor, char expectedControlByte);

/**
* Sends a given packet and waits for unumberd acknowledge
*/
int sendPacketAndWaitResponse(int fileDescriptor, char* packet, char responseByte);
