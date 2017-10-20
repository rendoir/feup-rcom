#include <strings.h>
#include "serialPort.h"

struct termios old_tio, new_tio;

/*------------------------------------*/
/*------------------------------------*/
/*------------SERIAL PORT-------------*/
/*------------------------------------*/
/*------------------------------------*/

int init_inputMode(int caller){
    //Init input mode
    bzero(&new_tio, sizeof(new_tio));
    new_tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    new_tio.c_iflag = IGNPAR;
    new_tio.c_oflag = 0;
    new_tio.c_lflag = 0;
    if(caller == TRANSMITTER){
      new_tio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
      new_tio.c_cc[VMIN]     = 0;   /* Polling Mode*/
    }
    else if(caller == RECEIVER){
      new_tio.c_cc[VTIME]    = 0;   /* read time-out */
      new_tio.c_cc[VMIN]     = 1;   /* number chars to read (blocking) */
    }
    return 0;
  }

  int serialPort_setNewSettigns(int sp_fd, int caller){
    //Load original termios
    if ( tcgetattr(sp_fd,&old_tio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    init_inputMode(caller);
    tcflush(sp_fd, TCIOFLUSH);

    //Set new termios
    if ( tcsetattr(sp_fd,TCSANOW,&new_tio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    return 0;
  }

  int openSerialPort(int port, int caller){
    //Read command line arguments
    char serialName[255] = "/dev/ttyS";
    char str_port[2];
    sprintf(str_port, "%d", port);
    strcat(serialName, str_port);
    if (strcmp("/dev/ttyS0", serialName) && strcmp("/dev/ttyS1", serialName))
    {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    //Open serial port device for reading and writing and not as controlling tty
    //because we don't want to get killed if linenoise sends CTRL-C.
    int sp_fd;
    sp_fd = open(serialName, O_RDWR | O_NOCTTY );
    if (sp_fd <0) {
      perror(serialName);
      exit(-1);
    }
    return sp_fd;
  }


  int closeSerialPort_and_SetOldSettigns(const int* sp_fd){
    //Reset termios to the original
    if ( tcsetattr(*sp_fd,TCSANOW,&old_tio) == -1) {
          close(*sp_fd);
      perror("tcsetattr");
      exit(-1);
    }

    //Close serial port
    close(*sp_fd);
    return 0;
  }
