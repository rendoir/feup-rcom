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
	if (parseURL(argv[1], url) != 0){
		return -1;	
	}
	if (getIpByHost(url)) {
		printf("ERROR: Cannot find ip to hostname %s.\n", url->host);
		return -1;
	}
	printf("\nThe IP received to %s was %s\n", url->host, url->ip);
	ftp_connect(ftp, url->ip, url->port);
	const char* user = (strlen(url->user) != 0) ? url->user : "anonymous";
	char* password;
	if (strlen(url->password) != 0) {
		password = url->password;
	} else {
		password="guest";
	}
	if (ftp_login(ftp, user, password)) {
		printf("ERROR: Cannot login user %s\n", user);
		return -1;
	}
	if (ftp_cwd(ftp, url->directory)) {
		printf("ERROR: Cannot change directory to the folder of %s\n",url->file);
		return -1;
	}
	if (ftp_pasv(ftp)) {
		printf("ERROR: Cannot entry in passive mode\n");
		return -1;
	}
	ftp_retr(ftp, url->file);
	ftp_download(ftp, url->file);
	ftp_disconnect(ftp);

	return 0;
}
