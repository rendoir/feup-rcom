#pragma once

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//type to use inside URL struct
typedef char url_content[256];

// Struct that holds information about an URL
typedef struct{
	url_content user; 
	url_content password; 
	url_content host; 
	url_content ip;
	url_content path;
	url_content filename;
	int port;
}URL;

/**
 * Initialize an url struct
 * */
void initURL(URL* url);
/**
 * Parse a url given as url_str and fill the URL struct with the info.
 * */
int parseURL(URL* url, const char* url_str);

/**
 * Sets url->ip. url->host should be defined previously.
 * */
int getIpByHost(URL* url);

/**
 * Cuts the string by the first occurrence of chr. Str is set to be the tail.
 * */
char* getStringToDelimiter(char* str, char delimiter);
