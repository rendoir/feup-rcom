#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int parseProtocol(char *url, char *protocol);

int parseUser(char *url, char *user, int i);

int parsePassword(char *url, char *password, int i);

int parseHost(char *url, char *host, int i);

int parsePort(char *url, int *port, int i);

int parsePath(char *url, char *path, int i);

int parseURL(char *url);