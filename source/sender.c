#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "dataLink.h"


int main(int argc, char** argv)
{
  signal(SIGALRM, alarm_handler);  // Install alarm handler

  int port = atoi(argv[1]);
    if(port != 0 && port != 1){
      return -1;
  }

  int sp_fd; // Serial port file descriptor
  sp_fd = llopen(port,TRANSMITTER);
	if (sp_fd < 0){
		perror("Error in llopen");
		exit(-1);
  }

	char* myBuffer = "ola";
	if (llwrite(sp_fd,myBuffer,4) != 0){
		perror("Error writing data to serial port");
		exit(-1);
	}
  if(llclose(sp_fd,TRANSMITTER) != 0){
		perror("Error in llclose");
    exit(-1);
  }

  return 0;
}
