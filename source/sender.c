#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include "dataLink.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"


int main(int argc, char** argv)
{

  int sp_fd; // Serial port file descriptor
  struct termios old_tio, new_tio;
  signal(SIGALRM, alarm_handler);  // Install alarm handler

  //Read command line arguments
  char serialName[255] = "/dev/ttyS";
  if(argv[1] != 0){
    strcat(serialName, argv[1]);
  }
  else{
    printf("Argv[1] is null\n");
  }
  if ( (argc < 2) ||
  	     (strcmp("/dev/ttyS0", serialName) &&
  	      strcmp("/dev/ttyS1", serialName))){
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
  }

  //Open serial port device for reading and writing and not as controlling tty
  //because we don't want to get killed if linenoise sends CTRL-C.
  sp_fd = open(serialName, O_RDWR | O_NOCTTY );
  if (sp_fd <0) {
    perror(serialName);
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
  new_tio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
  new_tio.c_cc[VMIN]     = 0;   /* Polling Mode*/
  tcflush(sp_fd, TCIOFLUSH);

  //Set new termios
  if ( tcsetattr(sp_fd,TCSANOW,&new_tio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

	if (llopen(sp_fd,TRANSMITTER) == -1){
		perror("Error in llopen");
		exit(-1);
	}
	char* myBuffer = "ola";
	if (llwrite(sp_fd,myBuffer,4) != 0){
		perror("Error writing data to serial port");
		exit(-1);
	}
  llclose(sp_fd,TRANSMITTER);

  //Reset termios to the original
  if ( tcsetattr(sp_fd,TCSANOW,&old_tio) == -1) {
		close(sp_fd);
    perror("tcsetattr");
    exit(-1);
  }

  //Close serial port
  close(sp_fd);
  return 0;
}
