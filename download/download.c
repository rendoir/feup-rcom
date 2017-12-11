#include <stdio.h>
#include "parser.h"
#include "connection.h"

static void printUsage(char* argv0);

int main(int argc, char** argv) {
	URL *url = malloc(sizeof(URL));
	Connection *ftp = malloc(sizeof(Connection));
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

	
	ftp_connect(ftp, url->ip, url->port);

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
	if (ftp_login(ftp, user, password)) {
		printf("ERROR: Cannot login user %s\n", user);
		return -1;
	}


	// Changing directory
	if (ftp_cwd(ftp, url->directory)) {
		printf("ERROR: Cannot change directory to the folder of %s\n",
				url->file);
		return -1;
	}

	// Entry in passive mode
	if (ftp_pasv(ftp)) {
		printf("ERROR: Cannot entry in passive mode\n");
		return -1;
	}

	// Begins transmission of a file from the remote host
	ftp_retr(ftp, url->file);

	// Starting file transfer
	ftp_download(ftp, url->file);

	// Disconnecting from server
	ftp_disconnect(ftp);

	return 0;
}

void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
			argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}
