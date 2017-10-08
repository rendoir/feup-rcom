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
#include <signal.h>

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

int flag=1, conta=1;
/**
* Atende Alarme
*/
void atende()
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

int main(int argc, char** argv)
{
  int sp_fd;
  int res;
  int state = 0;
  struct termios old_tio, new_tio;
  unsigned char set[5];

  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

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
  new_tio.c_cc[VMIN]     = 0;   /* Polling Mode*/
  tcflush(sp_fd, TCIOFLUSH);

  //Set new termios
  if ( tcsetattr(sp_fd,TCSANOW,&new_tio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  while(conta < 4){
    if(flag){
       alarm(3);                 // activa alarme de 3s
       flag=0;
			 state = 0;
			 //Try to send message
			 res = write(sp_fd, set, 5);
			 printf("Try number %d, %d bytes written\n", conta, res);
       //Read from serial port
      unsigned char read_char;
      unsigned char package_received[5];
      while(state != 5 && flag == 0) {
        res = read(sp_fd, &read_char, 1);
        if (res > 0){
          printf("current state = %d\n",state);
          printf("read byte: 0x%x\n", read_char);
          switch(state){
            case 0: {
              if (read_char == FLAG){
                package_received[0] = read_char;
                state = 1;
              }
              break;
            }
            case 1: {
              if (read_char == A) {
                package_received[1] = read_char;
                state = 2;
              } else if (read_char != FLAG) {
                state = 0;
              }
              break;
            }
            case 2: {
              if (read_char == C_UA) {
                package_received[2] = read_char;
                state = 3;
              } else if (read_char == FLAG) {
                state = 1;
              } else {
                state = 0;
              }
              break;
            }
            case 3: {
              if (read_char == (package_received[1]^package_received[2])) {
                state = 4;
              } else {
                state = 0;
              }
              break;
            }
            case 4: {
              if (read_char == FLAG) {
                state = 5;
              } else {
                state = 0;
              }
              break;
            }
          }
        }

      }
      if (state == 5){
        printf("Acknowledge received\n");
				break;
      }
    }
 }



  //Reset termios to the original
  if ( tcsetattr(sp_fd,TCSANOW,&old_tio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  //Close serial port
  close(sp_fd);
  return 0;
}
