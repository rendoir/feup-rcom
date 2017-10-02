#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* To use read function */

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x01

unsigned char g_ua[5];




int main(int argc, char** argv){
	int state = 0;
	unsigned char c;
	unsigned char package_received[5];
	int fd,res;
    struct termios oldtio,newtio;
    g_ua[0] = FLAG;
	g_ua[1] = A;
	g_ua[2] = C_UA;
	g_ua[3] = g_ua[1]^g_ua[2];

    char serialName[255] = "/dev/ttyS";
    if(argv[1]!= 0) strcat(serialName, argv[1]);

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


    fd = open(serialName, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(serialName); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
   	printf("Waiting for some read\n");
	
	
	while(state != 5){

		res = read(fd,&c,1);
		printf("current state = %d\n",state);
		printf("read byte: 0x%x, num bytes= %d\n",c,res);
		switch(state){
			case 0:
			if (c == FLAG){
				package_received[0]=FLAG;
				state=1;
			}
			break;
			case 1:
			if (c == A){
				package_received[1]=A;
				state=2;
			}else if (c != FLAG) {
				state = 0;
			}
			break;
			case 2:
			if (c == C_SET){
				package_received[2]=C_SET;
				state = 3;
			}else if (c == FLAG){
				state = 1;
			}else{
				state = 0;
			}
			break;
			case 3:
			if (c == (package_received[1]^package_received[2])){
				state = 4;
			}else {
				state = 0;
			}
			break;
			case 4:
			if (c == FLAG){
				state = 5;
			}else{
				state = 0;
			}
			break;
		}
	}
	printf("Received SET\n");
	printf("Sending acknowledge");
	return 0;

}
