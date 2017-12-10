#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>


typedef struct URL{
  char protocol[32];
  char user[64];
  char password[64];
  char host[128];
  int port;
  char directory[256];
  char ip[64];
  char file[256];
}URL;


int parseProtocol(char *url, char *protocol);

int parseUser(char *url, char *user, int i);

int parsePassword(char *url, char *password, int i);

int parseHost(char *url, char *host, int i);

int parsePort(char *url, int *port, int i);

int parsePath(char *url, char *directory,char *file, int i);

int parseURL(char *url, URL* url_struct);

int initUrlStruct(URL* url_struct);

int getIpByHost(URL* url);