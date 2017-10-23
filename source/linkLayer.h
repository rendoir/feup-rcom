#ifdef LINK_LAYER_H
#define LINK_LAYER_H

/**
* Alarm Handler
*/
void alarm_handler();

/**
* Opens/establish the connection.
* caller - Who called the function: RECEIVER or TRANSMITTER
* Return: 0 if success, -1 if error.
*/
int llopen(char* port, int caller);

/**
* Opens/establish the connection.
* when caller is TRANSMITTER
* Return: 0 if success, negative on error.
*/
int llopenTransmitter(int fileDescriptor);

/**
* Opens/establish the connection.
* when caller is RECEIVER
* Return: 0 if success, negative on error.
*/
int llopenReceiver(int fileDescriptor);

#endif // LINK_LAYER_H
