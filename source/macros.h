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

#endif
