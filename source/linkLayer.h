#ifdef LINK_LAYER_H
#define LINK_LAYER_H


/**
* Alarm Handler
*/
void alarm_handler();

/**
* Builds a control packet and returns it on the frame parameter.
* Frame memory should be allocated previously.
*/
void buildControlFrame(char* frame, char address_field, char control_field);

/**
* Creates an Information Frame.
*/
void buildDataFrame(char** frame, char address_field, char control_field, char* data, int data_size);

/**
* Does byte stuffing on data.
* data_size is updated as reallocs are made.
*/
void byteStuffing(char** data, int* data_size);

/**
* Does byte unstuffing on data.
* data_size is updates as reallocs are made.
*/
void byteUnstuffing(char** data, int* data_size);

/**
* Opens/establishes the connection.
* caller - Who called the function: SENDER or RECEIVER
* Return: 0 if success, -1 if error.
*/
int llopen(char* port, int caller);

/**
* Opens/establish the connection.
* when caller is SENDER
* Return: 0 if success, negative on error.
*/
int llopenSender(int sp_fd);

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llopenReceiver(int fileDescriptor);

/**
* Builds the data frame.
* Sends the data frame until Receiver Ready or MAX_TRIES are achieved.
*/
int llwrite(int sp_fd, char* data, int data_size);

/**
* Closes the connection.
* Caller - SENDER or RECEIVER
* 0 if Success; -1 if Error.
*/
int llclose(int sp_fd, int caller);

int llcloseSender(int sp_fd);

int llcloseReceiver(int sp_fd);

#endif // LINK_LAYER_H
