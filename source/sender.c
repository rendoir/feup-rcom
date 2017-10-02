/**
  * Sender
**/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE  1

#define FLAG   0x7e
#define A      0X03
#define C_SET  0x03
#define C_DISC 0x0B
#define C_UA   0x07

volatile int STOP = FALSE;

int main(int argc, char** argv)
{
    int sp_fd, res;
    struct termios old_tio,new_tio;
    char input_buffer[255];
    char read_buffer[255];
    unsigned char set[5];

    //Read command line arguments
    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    //Init the set buffer
    set[0] = FLAG;
    set[1] = A;
    set[2] = C_SET;
    set[3] = set[1]^set[2];
    set[4] = FLAG;

    //Open serial port device for reading and writing and not as controlling tty
    //because we don't want to get killed if linenoise sends CTRL-C.

    sp_fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (sp_fd <0) {
      perror(argv[1]);
      exit(-1);
    }

    //Load original termios
    if ( tcgetattr(sp_fd,&old_tio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    //Init input mode
    bzero(&new_tio, sizeof(new_tio));
    new_tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    new_tio.c_iflag = IGNPAR;
    new_tio.c_oflag = 0;
    new_tio.c_lflag = 0;
    new_tio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    new_tio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
    tcflush(sp_fd, TCIOFLUSH);

    //Set new termios
    if ( tcsetattr(sp_fd,TCSANOW,&new_tio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //Write to serial port
    res = write(sp_fd, set, 5);
    printf("%d bytes written\n", res);

    //Read from serial port
    int  read_chars = 0;
    char single_char;
    while(read_chars < 5) {
      res = read(sp_fd, &single_char, 1);
      if (res <= 0) {
        perror("read");
        exit(-1);
      }
      read_buffer[read_chars++] = single_char;
    }

    //Reset termios to the original
    if ( tcsetattr(sp_fd,TCSANOW,&old_tio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(sp_fd);
    return 0;
}
