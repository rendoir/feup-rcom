#include <stdio.h>
#include "parser.h"
#include "connection.h"

void print_usage() {
	printf("\nUsage: download ftp://[user[:password]@]host[:port]/path\n");
}

int main(int argc, char** argv) {
	URL *url = malloc(sizeof(URL));
	Connection *ftp = malloc(sizeof(Connection));
	if (argc != 2) {
		print_usage();
		return 1;
	}
	if (parseURL(argv[1], url)) {
		printf("[ERROR] Failed to parse URL.\n");
		return -1;	
	}
	if (getIpByHost(url)) {
		printf("[ERROR] Couldn't find IP by hostname %s.\n", url->host);
		return -1;
	}
	printf("\nHOST:%s\nIP:%s\n", url->host, url->ip);
	if(ftp_connect(ftp, url->ip, url->port)) {
		printf("[ERROR] Couldn't connect\n");
		return -1;
	}
	const char* user = (strlen(url->user) != 0) ? url->user : "anonymous";
	char* password;
	if (strlen(url->password) != 0) {
		password = url->password;
	} else {
		password="guest";
	}
	if (ftp_login(ftp, user, password)) {
		printf("[ERROR] Couldn't login user %s:%s\n", user, password);
		return -1;
	}
	if (ftp_cwd(ftp, url->directory)) {
		printf("[ERROR] Couldn't change directory to %s\n", url->directory);
		return -1;
	}
	if (ftp_pasv(ftp)) {
		printf("[ERROR] Couldn't enter passive mode\n");
		return -1;
	}
	if(ftp_retr(ftp, url->file)) {
		printf("[ERROR] Couldn't retrieve file %s\n", url->file);
		return -1;
	}
	if(ftp_download(ftp, url->file)) {
		printf("[ERROR] Couldn't download file %s\n", url->file);
		return -1;
	}
	if(ftp_disconnect(ftp)) {
		printf("[ERROR] Couldn't disconnect.\n");
		return -1;
	}

	return 0;
}
