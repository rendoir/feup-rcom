#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"

#define TRANSMITTER 0
#define RECEIVER 1

/**
 * Init the struct termios to our inputMode,
 * depending on caller if is TRANSMITTER or RECEIVER
 */

int init_inputMode(struct termios *new_tio, int caller);

//TODO: comment
int serialPort_setNewSettigns(int sp_fd, int caller);

//TODO: comment
int openSerialPort(int port, int caller);

//TODO: comment
int closeSerialPort_and_SetOldSettigns(const int *sp_fd);

#endif