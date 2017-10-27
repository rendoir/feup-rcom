#include "utils.h"

void insertValueAt(unsigned char value, unsigned char *array, int index, unsigned long *array_size)
{
  int i;
  for (i = (*array_size) - 1; i > index; i--)
  {
    array[i + 1] = array[i];
  }
  array[index] = value;
  (*array_size)++;
}

void removeValueAt(unsigned char *array, int index, unsigned long *array_size)
{
  int i;
  for (i = index; i < (*array_size) - 1; i++)
  {
    array[i] = array[i + 1];
  }
  (*array_size)--;
}

void logToFile(const char *stringToLog){
  #ifdef DEBUG
  FILE *log_file = fopen(".log", "a+");
  if(log_file != NULL){
    time_t now;
    time(&now);

    struct tm* now_tm;
    now_tm = localtime(&now);

    char out[80];
    strftime (out, 80, "%Y-%m-%d %H:%M:%S", now_tm);
    char logString[1024];
    sprintf(logString, "%s : Mode: %s - %s\n", out, mode, stringToLog);
    fprintf(log_file, logString);
  }
  #endif
}