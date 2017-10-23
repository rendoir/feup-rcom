#include "applicationLayer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  ApplicationLayer app;
  initApp(&app, argc, argv);
  readFileData(&app);
  printFileData(&app);
  return 0;
}

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

int readFileData(ApplicationLayer *app) {
  FILE *file_ptr;
  long long file_length;

  file_ptr = fopen(app->file_path, "rw");
  fseek(file_ptr, 0, SEEK_END);
  file_length = ftell(file_ptr);
  rewind(file_ptr);

  app->file_data = malloc((file_length + 1));
  fread(app->file_data, file_length, 1, file_ptr);
  fclose(file_ptr);
  app->file_data[file_length] = '\0';

  return 0;
}

int printFileData(ApplicationLayer *app) {
  printf("Print\n");
  printf("Data: %s\n", app->file_data);

  return 0;
}

void printUsage() {
  printf("Usage: app port_number mode [file_path]\n");
  printf("    port_number: the serial port number to use\n");
  printf("    mode: \"sender\" or \"receiver\"\n");
  printf("    [file_path]: needed if the app works as a sender\n");
  exit(1);
}
