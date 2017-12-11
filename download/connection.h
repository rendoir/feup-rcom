#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> //for bzero
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct Connection
{
    int control_socket_fd; // file descriptor to control socket
    int data_socket_fd; // file descriptor to data socket
} Connection;

// get tcp socket
int connect_socket(const char* ip, int port);
//
int ftp_connect(Connection* ftp, const char* ip, int port);
//
int ftp_disconnect(Connection* ftp);
//
int ftp_download(Connection* ftp, const char* filename);
//
int ftp_cwd(Connection* ftp, const char* path);
//
int ftp_login(Connection* ftp, const char* user, const char* password);
//
int ftp_pasv(Connection* ftp);
//
int ftp_read(Connection* ftp, char* str, size_t size);
//
int ftp_retr(Connection* ftp, const char* filename);
//
int ftp_send(Connection* ftp, const char* str, size_t size);




