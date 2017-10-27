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

  long long bytes_per_data_packet;

  //File
  char* file_path;
  char* file_data;
  long long file_size;
  long long bytes_processed;
} ApplicationLayer;

typedef struct {
  char* frame;
  char* file_name;
  int frame_size;
  long long file_size;
} ControlFrame;

typedef struct {
  char* frame;
  char* data;
  int frame_size;
} DataFrame;

//Common
int initApp(ApplicationLayer *app, int argc, char** argv);
void run(ApplicationLayer *app);
int initConnection(ApplicationLayer *app);
int closeConnection(ApplicationLayer *app);

//Sender
int readFileData(ApplicationLayer *app);
int send(ApplicationLayer *app);
void buildControlFrame(ApplicationLayer *app, ControlFrame *frame, char control);
void buildStartFrame(ApplicationLayer *app, ControlFrame *frame) { buildControlFrame(app, frame, CONTROL_START); }
void buildEndFrame(ApplicationLayer *app, ControlFrame *frame) { buildControlFrame(app, frame, CONTROL_END); }
void buildDataFrame(ApplicationLayer *app, DataFrame *frame);

//Receiver
int receive(ApplicationLayer *app);
void disassembleControlFrame(ApplicationLayer *app, ControlFrame *frame);
void disassembleDataFrame(ApplicationLayer *app, DataFrame *frame);
int writeFileData(ApplicationLayer *app);

//Common utils
void printUsage();
void printFileData(ApplicationLayer *app);
void freeControlFrame(ControlFrame *frame);
void freeDataFrame(DataFrame *frame);

#endif //APP_LAYER_H
