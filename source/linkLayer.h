#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include <stdlib.h>
#include <string.h> //used for memcpy
#include "serialPort.h"
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

/**
* Alarm Handler
*/
int alarm_handler();

/**
* Builds a control packet and returns it on the frame parameter.
* Frame memory should be allocated previously.
* Caller is usued to select the address value.
* If control field = SET | DISC | UA -> sequence_number should be -1;
*/
void buildControlFrame(char *frame, int caller, char control_field, long sequence_number);

/**
* Creates an Information Frame.
* It allocates space for the frame.
* Invokes byte stuffing.
* frame_size is updated with the new size of the created frame;
*/
void buildDataFrame(char **frame, char *data, int data_size, unsigned long *frame_size, long sequence_number);

/**
* Does byte stuffing on frame.
* frame_size is updated as reallocs are made.
*/
void byteStuffing(char **frame, unsigned long *frame_size);

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
unsigned char getAddress(int caller, char control_field);

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
* Builds the data frame.
* Sends the data frame until Receiver Ready or MAX_TRIES are achieved.
*/
int llwrite(int sp_fd, char *data, int data_size);

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
int readFrameHeader(int sp_fd, Frame_Header *expected_frame_header, int isData);

int processReply(int sp_fd, char address_expected, char expected_control_field);

/**
* Writes frame_to_write to sp_fd.
* Waits for reply and processes it.
* If reply control_field = C_RR | C_UA | C_DISC -> Ok, return 0;
* If num_tries >= MAX_TRIES -> return -1;
* If reply = C_REJ -> return -2 (should be called again).
*/
int writeAndReadReply(int sp_fd, char *frame_to_write, int frame_size, char expected_control_field);

int readAndWriteReply(int sp_fd, char *frame_to_write, int frame_size, char expected_control_field);

#endif // LINK_LAYER_H
