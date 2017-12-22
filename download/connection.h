#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct Connection
{
    int control_socket_fd; // file descriptor to control socket
    int passive_socket_fd; // file descriptor to passive socket
} Connection;

/**
 * Get tcp socket
 * */
int connect_socket(const char* ip, int port);

/**
 * Create Connection struct
 * Returns 0 if success, -1 otherwise.
 * */
int ftp_connect(Connection* ftp, const char* ip, int port);

/** Send QUIT to control socket
 * */
int ftp_disconnect(Connection* ftp);

/**
 * Read from passive socket
 * */
int ftp_download(Connection* ftp, const char* filename);

/**
 * Send CWD to control socket
 * */
int ftp_cwd(Connection* ftp, const char* path);

/**
 * Login in control socket
 * */
int ftp_login(Connection* ftp, const char* user, const char* password);

/**
 * Send PASV to control socket
 * Create new FTP connection to the passive socket
 * */
int ftp_pasv(Connection* ftp);

/**
 * Read from control socket
 * */
int ftp_read(Connection* ftp, char* str, size_t size);

/**
 * Send RETR file to control socket
 * */
int ftp_retr(Connection* ftp, const char* filename);

/**
 * Send string to control socket
 * */
int ftp_send(Connection* ftp, const char* str, size_t size);




