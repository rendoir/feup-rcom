#include <stdlib.h>
#include <string.h>


typedef struct Files{
  char name[128];
  char type[128];
  unsigned long length;
  char *bytes;
} File;

int makefile (File* file,const char* name,const char* type,const void* bytes){
   file = malloc(sizeof(File));
   if(file ==  NULL)
    return 1;
   memcpy(file->name, name, 128);
   memcpy(file->type, type, 128);
   file->length = sizeof(bytes);
   file->bytes = malloc(sizeof(bytes));
   if(file->bytes ==  NULL)
    return 1;
   file->length = sizeof(bytes);
   memcpy(file->bytes, bytes, file->length);
   return 0;
}
