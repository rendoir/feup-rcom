#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "macros.h"

/**
 * Init the struct termios to our inputMode,
 * depending on caller if is TRANSMITTER or RECEIVER
 */

int initInputMode(struct termios* new_tio, int caller);

//TODO: comment
int setNewSettings(int sp_fd, int caller);

//TODO: comment
int openSerialPort(char* port, int caller);

//TODO: comment
int closeSerialPort(const int *sp_fd);

#endif
