#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int parseProtocol(char *url, char *protocol) {
  //Go through until '://'
  int i = 0;
  int front_slash = 0;
  int colon = 0;
  while(!colon || front_slash < 2) {
    char current_char = url[i];
    if(current_char == '\0')
      break;
    if(current_char == ':')
      colon++;
    else if(current_char == '/')
      front_slash++;
    else protocol[i] = current_char;
    i++;
  }
  if(colon == 1 && front_slash == 2)
    return i;
  return -1;
}

int parseUser(char *url, char *user, int i) {
  //Go through until ':' or '@' or end of string
  int colon = 0;
  int at = 0;
  int j = 0;
  while(!at && !colon) {
    char current_char = url[i];
    if(current_char == '\0')
      break;
    if(current_char == ':')
      colon++;
    else if(current_char == '@')
      at++;
    else user[j] = current_char;
    j++;
    i++;
  }
  if(!at && !colon)
    return -1;
  return i;
}

int parsePassword(char *url, char *password, int i) {
  //Go through until '@' or end of string
  int at = 0;
  int j = 0;
  while(!at) {
    char current_char = url[i];
    if(current_char == '\0')
      break;
    if(current_char == '@')
      at++;
    else password[j] = current_char;
    j++;
    i++;
  }
  if(!at)
    return -1;
  return i;
}

int parseHost(char *url, char *host, int i) {
  //Go through until '/' or ':' or end of string
  int colon = 0;
  int front_slash = 0;
  int j = 0;
  while(!front_slash && !colon) {
    char current_char = url[i];
    if(current_char == '\0')
      break;
    if(current_char == ':')
      colon++;
    else if(current_char == '/')
      front_slash++;
    else host[j] = current_char;
    j++;
    i++;
  }
  if((!front_slash && !colon) || !strcmp(host, ""))
    return -1;
  return i;
}

int parsePort(char *url, int *port, int i) {
  //Go through until '/' or end of string
  int front_slash = 0;
  int j = 0;
  char port_string[32] = "";
  while(!front_slash) {
    char current_char = url[i];
    if(current_char == '\0')
      break;
    if(current_char == '/')
      front_slash++;
    else port_string[j] = current_char;
    j++;
    i++;
  }
  if(!front_slash)
    return -1;
  char *conversion_result;
  int converted_port = strtol(port_string, &conversion_result, 10);
  if(port_string == conversion_result)
    return -1;
  *port = converted_port;
  return i;
}

int parsePath(char *url, char *path, int i) {
  //Go through until end of string
  int j = 0;
  while(1) {
    char current_char = url[i];
    if(current_char == '\0')
      return i;
    path[j] = current_char;
    j++;
    i++;
  }
}

int parseURL(char *url) {
  char protocol[32] = "";
  char user[64] = "";
  char password[64] = "";
  char host[128] = "";
  int port = 21;
  char path[256] = "";

  int i;
  int j;
  if((i = parseProtocol(url, protocol)) < 0) {
    printf("Error parsing protocol");
    return -1;
  }

  if((j = parseUser(url, user, i)) < 0) {
    strcpy(user, "anonymous");
  } else {
    i = j;
    if((j = parsePassword(url, password, i)) < 0)
      strcpy(password, "");
    else i = j;
  }

  if((i = parseHost(url, host, i)) < 0) {
    printf("Error parsing host");
    return -1;
  }

  if((j = parsePort(url, &port, i)) > 0) {
    i = j;
  }

  parsePath(url, path, i);

  printf("--------------------------------------------------\n");
  printf("URL:%s\n", url);
  printf("Protocol:%s\n", protocol);
  printf("User:%s\n", user);
  printf("Password:%s\n", password);
  printf("Host:%s\n", host);
  printf("Port:%d\n", port);
  printf("Path:%s\n", path);
  printf("--------------------------------------------------\n");
  return 0;
}

int main() {
  parseURL("ftp://ftp.up.pt/");
  parseURL("ftp://@ftp.up.pt/");
  parseURL("ftp://user@ftp.up.pt/");
  parseURL("ftp://user:password@ftp.up.pt/");
  parseURL("ftp://user:password@ftp.up.pt:69/");
  parseURL("ftp://user:password@ftp.up.pt:69/path/to/file");
}
