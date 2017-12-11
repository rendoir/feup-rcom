#include "parser.h"



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
    else {
      user[j] = current_char;
    }
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

int parsePath(char *url, char *directory,char *file, int i) {
  //Go through until end of string
  int j = 0;
  int iterator = i-1;
  int lastSlashIndex = i-1;
  char current_char;
  // Get last slash index.
  while (1){
    current_char = url[iterator];
    if (current_char == '\0'){
      break;
    }
    if (current_char == '/'){
      lastSlashIndex = iterator;
    }
    iterator++;
  }
  while(i < lastSlashIndex) {
    directory[j] = url[i];
    j++;
    i++;
  }
  j = 0;
  lastSlashIndex++;
  while(lastSlashIndex < iterator){
    file[j] = url[lastSlashIndex];
    lastSlashIndex++;
    j++;
  }
  return 0;
}

int initUrlStruct(URL* url_struct){
  memset(url_struct->protocol,'\0',sizeof(url_struct->protocol));
  memset(url_struct->user, '\0', sizeof(url_struct->user));
  memset(url_struct->password, '\0', sizeof(url_struct->password));
  memset(url_struct->host, '\0', sizeof(url_struct->host));
  url_struct->port = 21;
  memset(url_struct->directory, '\0', sizeof(url_struct->directory));
  memset(url_struct->file, '\0', sizeof(url_struct->file));
  return 0;
}

int parseURL(char *url, URL* url_struct) {
  int i;
  int j;
  initUrlStruct(url_struct);
  
  if((i = parseProtocol(url, url_struct->protocol)) < 0) {
    printf("Error parsing protocol");
    return -1;
  }

  if((j = parseUser(url, url_struct->user, i)) < 0) {
    strcpy(url_struct->user, "anonymous");
  } else {
    i = j;
    if((j = parsePassword(url, url_struct->password, i)) < 0)
      strcpy(url_struct->password, "");
    else i = j;
  }

  if((i = parseHost(url, url_struct->host, i)) < 0) {
    printf("Error parsing host");
    return -1;
  }

  if((j = parsePort(url, &(url_struct->port), i)) > 0) {
    i = j;
  }

  parsePath(url, url_struct->directory, url_struct->file, i);

  printf("--------------------------------------------------\n");
  printf("URL:%s\n", url);
  printf("Protocol:%s\n", url_struct->protocol);
  printf("User:%s\n", url_struct->user);
  printf("Password:%s\n", url_struct->password);
  printf("Host:%s\n", url_struct->host);
  printf("Port:%d\n", url_struct->port);
  printf("Directory:%s\n", url_struct->directory);
  printf("File:%s\n", url_struct->file);
  printf("--------------------------------------------------\n");
  return 0;
}

int getIpByHost(URL* url) {
	struct hostent* h;

	if ((h = gethostbyname(url->host)) == NULL) {
		perror("gethostbyname()");
		return -1;
	}
	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(url->ip, ip);

	return 0;
}

/*
int main() {
  URL* url_struct = malloc(sizeof(URL));
  parseURL("ftp://demo:password@test.rebex.net:21/readme.txt",url_struct);
  parseURL("ftp://ftp.up.pt/",url_struct);
  parseURL("ftp://user@ftp.up.pt/",url_struct);
  parseURL("ftp://user:password@ftp.up.pt/",url_struct);
  parseURL("ftp://user:password@ftp.up.pt:69/",url_struct);
  parseURL("ftp://user:password@ftp.up.pt:69/directory/to/file",url_struct);
  parseURL("ftp://user:password@ftp.up.pt/dir1/dir2/file.txt",url_struct);
}
*/