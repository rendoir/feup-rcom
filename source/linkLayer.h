#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include <stdlib.h>
#include <string.h> //used for memcpy
#include "serialPort.h"
#include "macros.h"

#define MAX_SIZE 1024

typedef struct {
    char port[20];
    int baudRate;
    unsigned int sequenceNumber;
    unsigned int timeout;
    unsigned int numTransmissions;
    char frame[MAX_SIZE];
} LinkLayer;

typedef struct {
	char address_field;
	char control_field;
	char bcc1;
	char* data;
	char bcc2;
} DataStruct;

typedef struct {
	char address_field;
	char control_field;
	char bcc1;
} ControlStruct;

typedef enum {START,FLAG_REC,A_REC,C_REC,BCC1_OK,READ_DATA,BCC2_OK,STOP} Data_State;

typedef enum {START,FLAG_REC,A_REC,C_REC,BCC_OK,STOP} Control_State;

/**
* Alarm Handler
*/
void alarm_handler();

/**
* Builds a control packet and returns it on the frame parameter.
* Frame memory should be allocated previously.
* Caller is usued to select the address value.
*/
void buildControlFrame(char *frame, int caller, char control_field, long sequence_number);

/**
* Creates an Information Frame.
* It allocates space for the frame.
* Invokes byte stuffing.
* frame_size is updated with the new size of the created frame;
*/
void buildDataFrame(char **frame, char *data, int data_size, int *frame_size, long sequence_number);

/**
* Does byte stuffing on frame.
* frame_size is updated as reallocs are made.
*/
void byteStuffing(char **frame, int *frame_size);

/**
* Does byte unstuffing on frame.
* frame_size is updates as reallocs are made.
*/
void byteUnstuffing(char **frame, int *frame_size);

/**
 * Returns the Address that should be used on a control frame.
 * caller - Who called the function: SENDER or RECEIVER
 * Returns: The address that should be used.
 * */
unsigned char getAddress(int caller, char control_field);

/**
 * Returns the Block Check Character of the data array.
 * */
unsigned char getBCC(char *data, int data_size);

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
* State Machine that analysis control frames received.
* Returns 0 in case of success.
* control_struct is filled with the frame read.
*/
int readControlFrame(int sp_fd, char expected_control_field, ControlStruct *control_struct);

/*
* State machine that analysis data frames received.
* data_struct is filled with the frame read only if no errors detected and not duplicated.
* Returns: 0 if no errors detected or if errors detected but duplicated -> should trigger a RR.
* -1 if error in bcc2 -> should trigger a REJ.
*/
int readDataFrame(int sp_fd, char expected_seq_number, DataStruct *data_struct);

/**
* Writes frame_to_write to sp_fd.
* Waits for reply and processes it.
* If reply control_field = C_RR | C_UA | C_DISC -> Ok, return 0;
* If num_tries >= MAX_TRIES -> return -1;
* If reply = C_REJ -> return -2 (should be called again).
*/
int writeAndReadReply(int sp_fd, char* frame_to_write, int frame_size);



#endif // LINK_LAYER_H
