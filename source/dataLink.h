#ifndef DATA_LINK_H
#define DATA_LINK_H

#include "state_machine.h"
#include "serialPort.h"
#include "string.h"

#define FLAG 0x7e // delimiter flag
#define A 0X03    // address

#define C_SET 0x03  // set up
#define C_DISC 0x0B // disconnect
#define C_UA 0x07   // unnumbered acknowledgment
#define C_RR 0x05   // receiver ready / positive acknowledge
#define C_REJ 0x01  // reject / negative acknowledge

#define TRANSMITTER 0
#define RECEIVER 1

#define CP_LENGTH 5 // control frame length

#define ESCAPE_CHAR 0x7d

#define TIME_OUT 3
#define MAX_TRIES 4

/**
* Atende Alarme
*/
void alarm_handler();

/**
* Inserts a char in any given index of the array.
*/
void insertValueAtPos(size_t pos, char value, char *array, int length);

/**
 * Makes the Frame for sender/receiver
 */
void buildFrame(char *frame, char *buffer, unsigned length, unsigned char bcc2);

/**
* Makes a control/supervision frame with the given control byte/field.
*/
void buildControlFrame(char controlByte, char *frame);

/**
* Reads a control frame from the file descriptor and returns the current state.
* Returns the control frame read, or null if bcc fails;
*/
char readControlFrame(int fileDescriptor);

/**
* Sends a given frame and waits for unumberd acknowledge
*/
char sendControlFrameAndWait(int fileDescriptor, char *frame);

char sendInfoFrameAndWait(int fileDescriptor, char *frame, int sizeOfFrame);

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, -1 if error.
*/
int llopen(int fileDescriptor, int caller);

/**
* Opens/establish the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llopenTrasnmitter(int fileDescriptor);

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llopenReceiver(int fileDescriptor);

/*
 * Do Stuffing of Buffer
 */
int stuffingBuffer(String *bufferString, unsigned *size, char *bcc2);

/**
* Writes a buffer array to the fileDescriptor.
* length - Length of the array to send
* Note: The buffer will be processed with byte stuffing before being sent.
*/
int llwrite(int fileDescriptor, char *buffer, unsigned length);

/**
* A method to read from the file descriptor into the buffer (frame).
*/
int llread(int fd, char *frame);

/**
* Close the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llclose(int fileDescriptor, int caller);

/**
* Close the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llcloseTransmitter(const int *fileDescriptor, char *disc_frame);

/**
* Close the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llcloseReceiver(const int *fileDescriptor, char *disc_frame);

#endif
