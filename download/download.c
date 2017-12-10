#include <stdio.h>
#include "parser.h"
#include "ftp.h"

static void printUsage(char* argv0);

int main(int argc, char** argv) {
	URL *url = malloc(sizeof(URL));
	FTP *ftp = malloc(sizeof(FTP));
	if (argc != 2) {
		printUsage(argv[0]);
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

	
	ftpConnect(ftp, url->ip, url->port);

	// Verifying username
	const char* user = (strlen(url->user) != 0) ? url->user : "anonymous";

	// Verifying password
	char* password;
	if (strlen(url->password) != 0) {
		password = url->password;
	} else {
		password="guest";
	}

	// Sending credentials to server
	if (ftpLogin(ftp, user, password)) {
		printf("ERROR: Cannot login user %s\n", user);
		return -1;
	}


	// Changing directory
	if (ftpCWD(ftp, url->directory)) {
		printf("ERROR: Cannot change directory to the folder of %s\n",
				url->file);
		return -1;
	}

	// Entry in passive mode
	if (ftpPasv(ftp)) {
		printf("ERROR: Cannot entry in passive mode\n");
		return -1;
	}

	// Begins transmission of a file from the remote host
	ftpRetr(ftp, url->file);

	// Starting file transfer
	ftpDownload(ftp, url->file);

	// Disconnecting from server
	ftpDisconnect(ftp);

	return 0;
}

void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
			argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}
