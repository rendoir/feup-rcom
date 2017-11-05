#ifndef MACROS_H
#define MACROS_H

#define FLAG 0x7e

/**
* I FRAME, SET and DISC are Commands;
* UA,RR,REJ are Replies
*/

// Use A_SENDER_COMMAND for Commands sent by sender or Replies by receiver
#define A_SENDER_COMMAND 0x03
// Use A_RECEIVER_COMMAND for Commands sent by receiver or Replies sent by sender
#define A_RECEIVER_COMMAND 0x01

// Control field
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR 0x05
#define C_REJ 0x01

#define ESCAPE 0x7d
#define STUFF_XOR 0x20

#define SENDER 0
#define RECEIVER 1

#define BAUDRATE B115200 // Use values 2400 4800 9600 19200 28800 38400 57600 115200
#define DEFAULT_SERIAL_PATH "/dev/ttyS"

#define MAX_TRIES 3

#define ALARM_TIME 10

#define DATA_FRAME_MIN_SIZE 7

extern char *mode;

#endif
