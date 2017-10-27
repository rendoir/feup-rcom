#include "applicationLayer.h"
#include "linkLayer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static long serial_number = 0;

int main(int argc, char** argv) {
  ApplicationLayer app;
  initApp(&app, argc, argv);
  run(&app);
  return 0;
}

//Common
int initApp(ApplicationLayer *app, int argc, char** argv) {
  if(argc < 3 || argc > 5)
    printUsage();

  app->bytes_per_data_packet = 1024;
  app->bytes_processed = 0;
  app->port = malloc(12);
  sprintf(app->port, "/dev/ttyS%s", argv[1]);

  if(strcmp(argv[2], "sender") == 0)
    app->mode = SENDER;
  else if(strcmp(argv[2], "receiver") == 0)
    app->mode = RECEIVER;
  else printUsage();

  if(app->mode == SENDER) {
    if(argc >= 4) {
      app->file_path = malloc(NAME_MAX);
      strcpy(app->file_path, argv[3]);
    } else
      printUsage();
    if(argc > 4)
      app->bytes_per_data_packet = atoll(argv[4]);
  }

  return 0;
}

void run(ApplicationLayer *app){
  if(initConnection(app) < 0)
    exit(1);
  if(app->mode == SENDER) {
	  readFileData(app);
    printFileData(app);
    send(app);
  } else if (app->mode == RECEIVER) {
    receive(app);
    printFileData(app);
    writeFileData(app);
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
  if(file_ptr == NULL)
    exit(1);
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
  DataFrame	data_frame;
  buildStartFrame(app, &control_frame);
  llwrite(app->sp_fd, control_frame.frame, control_frame.frame_size);
  while (app->bytes_processed < app->file_size) {
	  buildDataFrame(app, &data_frame);
    llwrite(app->sp_fd, data_frame.frame, control_frame.frame_size);
  }
  buildEndFrame(app, &control_frame);
  llwrite(app->sp_fd, control_frame.frame, control_frame.frame_size);
  return 0;
}

void buildControlFrame(ApplicationLayer *app, ControlFrame *frame, char control) {
  frame->file_size = app->file_size;
  frame->file_name = app->file_path;
  int size_of_file_size = sizeof(frame->file_size) + 1;
  int size_of_file_name = strlen(frame->file_name) + 1;
  frame->frame = malloc(5 + size_of_file_size + size_of_file_name);
  frame->frame[0] = control;
  frame->frame[1] = TYPE_FILE_SIZE;
  frame->frame[2] = size_of_file_size;
  char* file_size_str = malloc(size_of_file_size);
  sprintf(file_size_str, "%08lld", frame->file_size);
  int j = 0;
  for (int i = 3; i < 3 + size_of_file_size; i++) {
	  frame->frame[i] = file_size_str[j];
	  j++;
  }
  free(file_size_str);
  frame->frame[3 + size_of_file_size] = TYPE_FILE_NAME;
  frame->frame[4 + size_of_file_size] = size_of_file_name;
  char* file_name_str = malloc(size_of_file_name);
  sprintf(file_name_str, "%s", frame->file_name);
  j = 0;
  for (int i = 5 + size_of_file_size; i < 5 + size_of_file_size + size_of_file_name; i++) {
	  frame->frame[i] = file_name_str[j];
	  j++;
  }
  free(file_name_str);
  frame->frame_size = 5 + size_of_file_size + size_of_file_name;

  //Debug
  printf("\nBuilt control frame:\n");
  for (int i = 0; i < 5 + size_of_file_size + size_of_file_name; i++)
	  printf("%d ", frame->frame[i]);
  printf("\n");

}

void buildDataFrame(ApplicationLayer *app, DataFrame *frame) {
	char control = CONTROL_DATA;
	char serial = serial_number++ % 255;
	long long bytes_left = app->file_size - app->bytes_processed;
	long long bytes_to_write;
	if (bytes_left < app->bytes_per_data_packet)
		bytes_to_write = bytes_left;
	else bytes_to_write = app->bytes_per_data_packet;
	char l2 = bytes_to_write / 256;
	char l1 = bytes_to_write % 256;
	frame->data = malloc(bytes_to_write);
	for (int i = 0; i < bytes_to_write; i++)
		frame->data[i] = app->file_data[app->bytes_processed++];
	frame->frame = malloc(4 + bytes_to_write);
	frame->frame[0] = control;
	frame->frame[1] = serial;
	frame->frame[2] = l2;
	frame->frame[3] = l1;
	for (int i = 4; i < 4 + bytes_to_write; i++)
		frame->frame[i] = frame->data[i - 4];
  frame->frame_size = 4 + bytes_to_write;

	//Debug
	printf("\nBuilt data frame:\n");
	for (int i = 0; i < 4 + bytes_to_write; i++)
		printf("%d ", frame->frame[i]);
	printf("\n");
}

//Receiver
int receive(ApplicationLayer *app) {
  ControlFrame control_frame;
  DataFrame	data_frame;
  llread(app->sp_fd, &control_frame.frame);
  disassembleControlFrame(app, &control_frame);
  while (app->bytes_processed < app->file_size) {
    llread(app->sp_fd, &data_frame.frame);
    disassembleDataFrame(app, &data_frame);
  }
  llread(app->sp_fd, &control_frame.frame);
  disassembleControlFrame(app, &control_frame);
  return 0;
}

void disassembleControlFrame(ApplicationLayer *app, ControlFrame *frame) {
  char control = frame->frame[0];
  char t1 = frame->frame[1];
  char l1 = frame->frame[2];
  char* v1 = malloc(l1);
  for(int i = 0; i < l1; i++)
    v1[i] = frame->frame[i + 3];
  app->file_size = atoll(v1);
  char t2 = frame->frame[3 + l1];
  char l2 = frame->frame[4 + l1];
  app->file_path = malloc(l2);
  for(int i = 0; i < l2; i++)
    app->file_path[i] = frame->frame[i + 5 + l1];

  if(control == CONTROL_START)
    app->file_data = malloc(app->file_size);

  //Debug
  printf("\nDisassembled control frame:\n");
  printf("%d %d %d ", control, t1, l1);
  for (int i = 0; i < l1; i++)
  	printf("%d ", v1[i]);
  printf("%d %d ", t2, l2);
  for (int i = 0; i < l2; i++)
  	printf("%d ", app->file_path[i]);
  printf("\n");

  free(v1);
}

void disassembleDataFrame(ApplicationLayer *app, DataFrame *frame) {
  char control = frame->frame[0];
  char serial = frame->frame[1];
  char l2 = frame->frame[2];
  char l1 = frame->frame[3];
  char data_size = l2 * 256 + l1;
  frame->data = malloc(data_size);
  for(int i = 0; i < data_size; i++){
    frame->data[i] = frame->frame[i + 4];
    app->file_data[app->bytes_processed++] = frame->data[i];
  }

  //Debug
  printf("\nDisassembled data frame:\n");
  printf("%d %d %d %d ", control, serial, l2, l1);
  for (int i = 0; i < data_size; i++)
  	printf("%d ", frame->data[i]);
  printf("\n");
}

int writeFileData(ApplicationLayer *app) {
  FILE *file_ptr;

  file_ptr = fopen(app->file_path, "w");
  if(file_ptr == NULL)
    exit(1);

  fwrite(app->file_data, app->file_size, 1, file_ptr);
  fclose(file_ptr);

  return 0;
}

//Common utils
void printUsage() {
  printf("Usage: app port_number mode [file_path]\n");
  printf("    port_number: the serial port number to use\n");
  printf("    mode: \"sender\" or \"receiver\"\n");
  printf("    [file_path]: needed if the app works as a sender\n");
  printf("    [bytes_per_data_packet]: defaults to 1024\n");
  exit(1);
}

void printFileData(ApplicationLayer *app) {
  printf("\nFile data sent:\n");
  for(int i = 0; i < app->file_size; i++)
    printf("%d ", app->file_data[i]);
  printf("\n");
}

void freeControlFrame(ControlFrame *frame) {
  free(frame->frame);
  free(frame->file_name);
}

void freeDataFrame(DataFrame *frame){
  free(frame->frame);
  free(frame->data);
}
