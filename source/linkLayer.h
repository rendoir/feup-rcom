#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include <stdlib.h>
#include <string.h> //used for memcpy
#include "serialPort.h"
#include "alarm.h"
#include "macros.h"
#include "utils.h"

#define MAX_SIZE 1024

typedef struct
{
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
} LinkLayer;

typedef struct
{
	char address_field;
	char control_field;
} Frame_Header;

typedef enum {
	START,
	FLAG_REC,
	A_REC,
	C_REC,
	BCC1_OK,
	STOP
} State;

typedef enum {
	OK,
	DUPLICATED,
	REJECTED,
	ERROR
} Reply_Status;

/**
* Alarm Handler
*/
int alarm_handler(int);

/**
* Builds a control packet and returns it on the frame parameter.
* Frame memory should be allocated previously.
* Caller is usued to select the address value.
* If control field = SET | DISC | UA -> sequence_number should be -1;
*/
void buildControlFrameLINK(unsigned char *frame, int caller, unsigned char control_field, long sequence_number);
/**
* Creates an Information Frame.
* It allocates space for the frame.
* Invokes byte stuffing.
* frame_size is updated with the new size of the created frame;
* Returns the new frame size.
*/
unsigned long buildDataFrameLINK(unsigned char **frame, unsigned char *data, int data_size, long sequence_number);

/**
* Does byte stuffing on frame.
* frame_size is updated as reallocs are made.
*/
void byteStuffing(unsigned char **frame, unsigned long *frame_size);

/**
* Does byte unstuffing on frame.
* frame_size is updates as reallocs are made.
*/
void byteUnstuffing(unsigned char **frame, unsigned long *frame_size);

/**
 * Returns the Address that should be used on a control frame.
 * caller - Who called the function: SENDER or RECEIVER
 * Returns: The address that should be used.
 * */
unsigned char getAddress(int caller, unsigned char control_field);

/**
 * Returns the Block Check Character of the data array.
 * */
unsigned char getBCC(unsigned char *data, int data_size);

/**
* Opens/establishes the connection.
* caller - Who called the function: SENDER or RECEIVER
* Return: file descriptor;
*/
int llopen(char *port, int caller);

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: file descriptor;
*/
int llopenReceiver(int fileDescriptor);

/**
* Opens/establish the connection.
* when caller is SENDER
* Return: file descriptor;
*/
int llopenSender(int sp_fd);

/**
 * Reads a data frame. Returns the size of the data read.
 *
 */
int llread(int sp_fd, unsigned char **data);

/**
* Builds the data frame.
* Sends the data frame until Receiver Ready or MAX_TRIES are achieved.
*/
int llwrite(int sp_fd, unsigned char *data, unsigned long data_size);

/**
* Closes the connection.
* Caller - SENDER or RECEIVER
* 0 if Success; -1 if Error.
*/
int llclose(int sp_fd, int caller);

/**
* Closes the connection. Sends DISC, Waits DISC, Sends UA.
*/
int llcloseSender(int sp_fd);

/**
* Closes the connection. Receives DISC, Sends DISC,Receives UA.
*/
int llcloseReceiver(int sp_fd);

/*
* State machine that analysis data frames received.
* data_struct is filled with the frame read only if no errors detected and not duplicated.
* does not return if error in bcc1, address or control fields.
* Returns: 0 if no errors detected or if errors detected in bcc2 but duplicated -> should trigger a RR.
* -1 if error in bcc2 -> should trigger a REJ.
*/
int readDataFrame(int sp_fd, Frame_Header *frame_header, unsigned char **data_unstuffed, unsigned long *data_size);

/*
* State machine that checks frame headers.
* Returns 0 if frame = expected frame.
* Returns 1 if frame duplicated with no errors.
* Returns 2 if it is a C_REJ frame not duplicated.
* If errors detected, it will not return;
*/
Reply_Status readFrameHeader(int sp_fd, Frame_Header *expected_frame_header, int isData);

/**
 * Read data from a file to an array.
 * Returns 0 in case of success, -1 if some error happens with malloc.
 * */
int readFromFileToArray(int sp_fd, unsigned char **data, unsigned long *data_size);

/**
 * Writes a frame to sp_fd, waits and reads the reply.
 * Returns 0 if read reply in less than MAX_TRIES. -1 otherwise.
 * */
int writeAndReadReply(int sp_fd, unsigned char *frame_to_write, unsigned long frame_size, unsigned char expected_control_field, int caller);

#endif // LINK_LAYER_H
