#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include <stdlib.h>
#include <string.h> //used for memcpy
#include "macros.h"

/**
* Alarm Handler
*/
void alarm_handler();

/**
* Builds a control packet and returns it on the frame parameter.
* Frame memory should be allocated previously.
* Caller is usued to select the address value.
*/
void buildControlFrame(char *frame, int caller, char control_field);

/**
* Creates an Information Frame.
* It allocates space for the frame.
* Invokes byte stuffing.
* frame_size is updated with the new size of the created frame;
*/
void buildDataFrame(char **frame, char *data, int data_size, int *frame_size);

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
* Inserts a value at a given position of the array.
* Increments array_size by 1;
*/
void insertValueAt(char value, char *array, int index, int *array_size);

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
* Removes a value at a given position of the array.
* Decrements array_size by 1;
*/
void removeValueAt(char *array, int index, int *array_size);

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

#endif // LINK_LAYER_H
