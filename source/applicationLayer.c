#include "applicationLayer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static long serial_number = 0;

int main(int argc, char** argv) {
  ApplicationLayer app;
  initApp(&app, argc, argv);
  run(&app);
  return 0;
}

//Common
int initApp(ApplicationLayer *app, int argc, char** argv) {
  if(argc < 3 || argc > 4)
    printUsage();

  app->port = malloc(12);
  sprintf(app->port, "/dev/ttyS%s", argv[1]);

  if(strcmp(argv[2], "sender") == 0)
    app->mode = SENDER;
  else if(strcmp(argv[2], "receiver") == 0)
    app->mode = RECEIVER;
  else printUsage();

  if(app->mode == SENDER) {
    if(argc == 4) {
      app->file_path = malloc(128);
      strcpy(app->file_path, argv[3]);
    } else
      printUsage();
  }

  return 0;
}

void printFileData(ApplicationLayer *app) {
  printf("File data:\n%s\n", app->file_data);
}

void printUsage() {
  printf("Usage: app port_number mode [file_path]\n");
  printf("    port_number: the serial port number to use\n");
  printf("    mode: \"sender\" or \"receiver\"\n");
  printf("    [file_path]: needed if the app works as a sender\n");
  exit(1);
}

void run(ApplicationLayer *app){
  if(initConnection(app) < 0)
    exit(1);
  if(app->mode == SENDER) {
    sendData(app);
  } else if (app->mode == RECEIVER) {
    receiveData(app);
  }
  if(closeConnection(app) < 0)
    exit(1);
}

int initConnection(ApplicationLayer *app) {
  app->sp_fd = llopen(app->port, app->mode);
  return app->sp_fd;
}

int closeConnection(ApplicationLayer *app) {
  return llclose(app->sp_fd, app->mode);
}

//Sender
int readFileData(ApplicationLayer *app) {
  FILE *file_ptr;
  long long file_length;

  file_ptr = fopen(app->file_path, "rb");
  fseek(file_ptr, 0, SEEK_END);
  file_length = ftell(file_ptr);
  rewind(file_ptr);

  app->file_data = malloc((file_length + 1));
  fread(app->file_data, file_length, 1, file_ptr);
  fclose(file_ptr);
  app->file_data[file_length] = '\0';
  app->file_size = file_length + 1;

  return 0;
}

int send(ApplicationLayer *app){
  ControlFrame control_frame;
  buildStartFrame(app, &control_frame);
}

void buildStartFrame(ApplicationLayer *app, ControlFrame *frame) {
  control_frame->control = CONTROL_START;
  control_frame->file_size = app->file_size;
  control_frame->file_name = app->file_path;
  int size_of_file_size = sizeof(control_frame->file_size) + 1;
  int size_of_file_name = strlen(control_frame->file_name);
  control_frame->frame = malloc(5 + size_of_file_size + size_of_file_name);
  control_frame->frame[0] = control_frame->control;
  control_frame->frame[1] = TYPE_FILE_SIZE;
  control_frame->frame[2] = size_of_file_size;
  char* file_size_str = malloc(size_of_file_size);
  sprintf(file_size_str, "%d", control_frame->file_size);
  int i;
  for(i = 3; i < 3 + size_of_file_size; i++) {
    //TODO
  }
}

//Receiver
int receive(ApplicationLayer *app) {
//TODO
}
