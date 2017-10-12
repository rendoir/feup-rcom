#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "application.h"

#define BAUDRATE B38400

enum read_state {
	initial,
	data,
	finish
};

int main(int argc, char** argv){
	int sp_fd; // serial port file descriptor
  struct termios oldtio, newtio;
  char serialName[255] = "/dev/ttyS";
  if(argv[1] != 0){
		strcat(serialName, argv[1]);
	}
  if ( (argc < 2) ||
  	     (strcmp("/dev/ttyS0", serialName) &&
  	      strcmp("/dev/ttyS1", serialName))){
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

   /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
   */
    sp_fd = open(serialName, O_RDWR | O_NOCTTY );
    if (sp_fd < 0) {
			perror(serialName);
			exit(-1);
		}

    if ( tcgetattr(sp_fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 0;   /* read time-out */
    newtio.c_cc[VMIN]     = 1;   /* number chars to read (blocking) */

    tcflush(sp_fd, TCIOFLUSH);

    if ( tcsetattr(sp_fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
		char* trama = malloc(1024 * sizeof(char));
		llopen(sp_fd,RECEIVER);
		llread(sp_fd, trama);

		return 0;
}
