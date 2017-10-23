#ifndef APP_LAYER_H
#define APP_LAYER_H

#define SENDER   0
#define RECEIVER 1

typedef struct {
  //Connection
  int sp_fd;           //Serial port file descriptor
  int mode;            //SENDER or RECEIVER
  char* port;          // /dev/ttySx

  //File
  char* file_path;
  char* file_data;
  long long file_size;
} ApplicationLayer;

//Common
int initApp(ApplicationLayer *app, int argc, char** argv);
void printFileData(ApplicationLayer *app);
void printUsage();
void run(ApplicationLayer *app);
int initConnection(ApplicationLayer *app);
int closeConnection(ApplicationLayer *app);

//Sender
int readFileData(ApplicationLayer *app);
int sendData(ApplicationLayer *app);

//Receiver
int receiveData(ApplicationLayer *app);

#endif //APP_LAYER_H
