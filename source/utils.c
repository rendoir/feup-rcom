#include "utils.h"

double getRandomFloat(const double max)
{
  return max * ((double)rand() / RAND_MAX);
}

void insertValueAt(unsigned char value, unsigned char *array, int index, unsigned long *array_size)
{
  int i;
  for (i = (*array_size) - 1; i > index -1; i--)
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
  int log_file = open(".log", O_CREAT | O_RDWR | O_APPEND);
  if(log_file >= 0){
    time_t now;
    time(&now);

    struct tm* now_tm;
    now_tm = localtime(&now);

    char out[80];
    strftime (out, 80, "%Y-%m-%d %H:%M:%S", now_tm);

    write(log_file, out, strlen(out));
    write(log_file, " : Mode: ", 9);
    write(log_file, mode, strlen(mode));
    write(log_file, " - ", 3);
    write(log_file, stringToLog, strlen(stringToLog));
    write(log_file, "\n", 1);
  }
  #endif
}
