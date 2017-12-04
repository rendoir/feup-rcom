#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

typedef struct
{
    int control_socket_fd; // file descriptor to control socket
    int data_socket_fd; // file descriptor to data socket
} FTP;

int ftpConnect(FTP* ftp, const char* ip, int port);
int ftpLogin(FTP* ftp, const char* user, const char* password);
int ftpCWD(FTP* ftp, const char* path);
int ftpPasv(FTP* ftp);
int ftpRetr(FTP* ftp, const char* filename);
int ftpDownload(FTP* ftp, const char* filename);
int ftpDisconnect(FTP* ftp);

int ftpSend(FTP* ftp, const char* str, size_t size);
int ftpRead(FTP* ftp, char* str, size_t size);

