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

typedef struct FTP
{
    int control_socket_fd; // file descriptor to control socket
    int data_socket_fd; // file descriptor to data socket
} FTP;

int ftp_connect(FTP* ftp, const char* ip, int port);
int ftp_disconnect(FTP* ftp);
int ftp_download(FTP* ftp, const char* filename);
int ftp_cwd(FTP* ftp, const char* path);
int ftp_login(FTP* ftp, const char* user, const char* password);
int ftp_pasv(FTP* ftp);
int ftp_read(FTP* ftp, char* str, size_t size);
int ftp_retr(FTP* ftp, const char* filename);
int ftp_send(FTP* ftp, const char* str, size_t size);




