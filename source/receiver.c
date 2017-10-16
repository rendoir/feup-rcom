#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "dataLink.h"

#define BAUDRATE B38400

enum read_state {
	initial,
	data,
	finish
};

int main(int argc, char** argv){
  int port = atoi(argv[1]);
  if(port != 0 && port != 1){
    exit(-1);
  }

	int sp_fd; // serial port file descriptor
  sp_fd = llopen(port,RECEIVER);
	if (sp_fd < 0){
		perror("Error in llopen");
    exit(-1);
  }
  
  char* trama = malloc(1024 * sizeof(char));
  if(llread(sp_fd, trama) != 0){
		perror("Error writing data to serial port");
    exit(-1);
  }
  if(llclose(sp_fd,RECEIVER) != 0){
		perror("Error in llclose");
    exit(-1);
  }

  return 0;
}
