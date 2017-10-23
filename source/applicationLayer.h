#ifndef APP_LAYER_H
#define APP_LAYER_H

#define SENDER   0
#define RECEIVER 1

#define CONTROL_DATA  1
#define CONTROL_START 2
#define CONTROL_END   3

#define TYPE_FILE_SIZE 0
#define TYPE_FILE_NAME 1

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

typedef struct {
  char* frame;
  char control;
  char* file_name;
  int file_size;
} ControlFrame;

typedef struct {
  char* frame;
  char control;
  int data_size;
  char* data;
} DataFrame;

//Common
int initApp(ApplicationLayer *app, int argc, char** argv);
void run(ApplicationLayer *app);
int initConnection(ApplicationLayer *app);
int closeConnection(ApplicationLayer *app);

//Sender
int readFileData(ApplicationLayer *app);
int send(ApplicationLayer *app);
void buildStartFrame(ApplicationLayer *app, ControlFrame *frame);

//Receiver
int receive(ApplicationLayer *app);

#endif //APP_LAYER_H
