#ifndef APP_LAYER_H
#define APP_LAYER_H

#define SENDER   0
#define RECEIVER 1

typedef struct {
  int sp_fd;           //Serial port file descriptor
  int mode;            //SENDER or RECEIVER
  char* port;          // /dev/ttySx
  char* file_path;     //Path of the file to send
  char* file_data;
} ApplicationLayer;

int initApp(ApplicationLayer *app, int argc, char** argv);
int readFileData(ApplicationLayer *app);
int printFileData(ApplicationLayer *app);
void printUsage();

#endif //APP_LAYER_H
