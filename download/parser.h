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

/**
 * @brief function to parse the protocol of URL string
 * 
 * @param url URL String
 * @param protocol return string of protocol
 * @return int index of analyzed URL string
 */
int parseProtocol(char *url, char *protocol);

/**
 * @brief function to parse the user of URL string
 * 
 * @param url URL String
 * @param user return string of user in URL String
 * @param i index where user start on URL string
 * @return int index of analyzed URL string
 */
int parseUser(char *url, char *user, int i);

/**
 * @brief function to parse the Password of URL string
 * 
 * @param url URL String
 * @param password return String of password in URL String
 * @param i index where password start on URL string
 * @return int index of analyzed URL string
 */
int parsePassword(char *url, char *password, int i);

/**
 * @brief function to parse the Host of URL string
 * 
 * @param url URL String
 * @param host return String of host in URL String
 * @param i index where Host start on URL string
 * @return int index of analyzed URL string
 */
int parseHost(char *url, char *host, int i);

/**
 * @brief function to parse the Port of URL string
 * 
 * @param url URL String
 * @param port return String of port in URL String
 * @param i index where Port start on URL string
 * @return int index of analyzed URL string
 */
int parsePort(char *url, int *port, int i);

/**
 * @brief function to parse the Path of the directory and name of file of URL string
 * 
 * @param url URL String
 * @param directory return String of path of file in URL String
 * @param file return String of file in URL String
 * @param i index where path of directory start on URL string
 * @return int index of analyzed URL string
 */
int parsePath(char *url, char *directory,char *file, int i);

/**
 * @brief function to parse the one URL string into a URL struct
 * 
 * @param url URL String
 * @param url_struct 
 * @return int < 0 if occur a error
 */
int parseURL(char *url, URL* url_struct);

/**
 * @brief init url_struct alocating enough memory to the struct 
 * 
 * @param url_struct 
 * @return int 
 */
int initUrlStruct(URL* url_struct);

/**
 * @brief function to complete ip atributte of struct getting Ip from URL
 * 
 * @param url 
 * @return int < 0 if occur a error 
 */
int getIpByHost(URL* url);