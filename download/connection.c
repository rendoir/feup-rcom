#include "connection.h"

/*
	struct sockaddr_in {
		sa_family_t    sin_family; // address family: AF_INET /
		in_port_t      sin_port;   // port in network byte order /
		struct in_addr sin_addr;   // internet address /
	};

	// Internet address.
	struct in_addr {
		uint32_t       s_addr;// address in network byte order
	};
 */
int connect_socket(const char* ip, int port) {
	int tcp_socket;
	struct sockaddr_in ip_socket_address;

	// initialize struct
	bzero((char*) &ip_socket_address, sizeof(ip_socket_address));
	ip_socket_address.sin_family = AF_INET;
	ip_socket_address.sin_addr.s_addr = inet_addr(ip);
	ip_socket_address.sin_port = htons(port); 

	// tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return -1;
	}

	// connect to the server
	if (connect(tcp_socket, (struct sockaddr *) &ip_socket_address, sizeof(ip_socket_address))
			< 0) {
		perror("connect()");
		return -1;
	}

	return tcp_socket;
}

int ftp_connect(Connection* ftp, const char* ip, int port) {
	int socketfd;
	char rd[1024]; // read array

	if ((socketfd = connect_socket(ip, port)) < 0) {
		printf("ERROR: Cannot connect socket.\n");
		return -1;
	}

	ftp->control_socket_fd = socketfd;
	ftp->data_socket_fd = 0;

	if (ftp_read(ftp, rd, sizeof(rd))) {
		printf("ERROR: ftp_read failure.\n");
		return -1;
	}

	return 0;
}

int ftp_login(Connection* ftp, const char* user, const char* password) {
	char user_string[256];
	char pass_string[256];

	// username
	sprintf(user_string, "USER %s\r\n", user);
	if (ftp_send(ftp, user_string, strlen(user_string))) {
		printf("ERROR: ftp_send failure.\n");
		return -1;
	}

	if (ftp_read(ftp, user_string, sizeof(user_string))) {
		printf(
				"ERROR: Access denied reading username response.\nftp_read failure.\n");
		return -1;
	}

	// password
	sprintf(pass_string, "PASS %s\r\n", password);
	if (ftp_send(ftp, pass_string, strlen(pass_string))) {
		printf("ERROR: ftp_send failure.\n");
		return -1;
	}

	if (ftp_read(ftp, pass_string, sizeof(pass_string))) {
		printf(
				"ERROR: Access denied reading password response.\nftp_read failure.\n");
		return -1;
	}

	return 0;
}

int ftp_cwd(Connection* ftp, const char* path) {
	char cwd[1024];

	sprintf(cwd, "CWD %s\r\n", path);
	if (ftp_send(ftp, cwd, strlen(cwd))) {
		printf("ERROR: Cannot send path to CWD.\n");
		return -1;
	}

	if (ftp_read(ftp, cwd, sizeof(cwd))) {
		printf("ERROR: Cannot send path to change directory.\n");
		return -1;
	}

	return 0;
}

int ftp_pasv(Connection* ftp) {
	char pasv[1024] = "PASV\r\n";
	if (ftp_send(ftp, pasv, strlen(pasv))) {
		printf("ERROR: Cannot enter in passive mode.\n");
		return -1;
	}

	if (ftp_read(ftp, pasv, sizeof(pasv))) {
		printf("ERROR: None information received to enter in passive mode.\n");
		return -1;
	}

	// starting process information
	int ipPart1, ipPart2, ipPart3, ipPart4;
	int port1, port2;
	if ((sscanf(pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,
			&ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0) {
		printf("ERROR: Cannot process information to calculating port.\n");
		return -1;
	}

	// cleaning buffer
	memset(pasv, 0, sizeof(pasv));

	// forming ip
	if ((sprintf(pasv, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4))
			< 0) {
		printf("ERROR: Cannot form ip address.\n");
		return -1;
	}

	// calculating new port
	int portResult = port1 * 256 + port2;

	printf("IP: %s\n", pasv);
	printf("PORT: %d\n", portResult);

	if ((ftp->data_socket_fd = connect_socket(pasv, portResult)) < 0) {
		printf(
				"ERROR: Incorrect file descriptor associated to ftp data socket fd.\n");
		return -1;
	}

	return 0;
}

int ftp_retr(Connection* ftp, const char* filename) {
	char retr[1024];

	sprintf(retr, "RETR %s\r\n", filename);
	if (ftp_send(ftp, retr, strlen(retr))) {
		printf("ERROR: Cannot send filename.\n");
		return -1;
	}

	if (ftp_read(ftp, retr, sizeof(retr))) {
		printf("ERROR: None information received.\n");
		return -1;
	}

	return 0;
}

int ftp_download(Connection* ftp, const char* filename) {
	FILE* file;
	int bytes;

	if (!(file = fopen(filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return -1;
	}

	char buf[1024];
	while ((bytes = read(ftp->data_socket_fd, buf, sizeof(buf)))) {
		if (bytes < 0) {
			printf("ERROR: Nothing was received from data socket fd.\n");
			return -1;
		}

		if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
			printf("ERROR: Cannot write data in file.\n");
			return -1;
		}
	}

	fclose(file);
	close(ftp->data_socket_fd);

	return 0;
}

int ftp_disconnect(Connection* ftp) {
	char disc[1024];

	if (ftp_read(ftp, disc, sizeof(disc))) {
		printf("ERROR: Cannot disconnect account.\n");
		return -1;
	}

	sprintf(disc, "QUIT\r\n");
	if (ftp_send(ftp, disc, strlen(disc))) {
		printf("ERROR: Cannot send QUIT command.\n");
		return -1;
	}

	if (ftp->control_socket_fd)
		close(ftp->control_socket_fd);

	return 0;
}

int ftp_send(Connection* ftp, const char* str, size_t size) {
	int bytes;

	if ((bytes = write(ftp->control_socket_fd, str, size)) <= 0) {
		printf("WARNING: Nothing was send.\n");
		return -1;
	}

	printf("Bytes send: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int ftp_read(Connection* ftp, char* str, size_t size) {
	FILE* fp = fdopen(ftp->control_socket_fd, "r");

	do {
		memset(str, 0, size);
		str = fgets(str, size, fp);
		printf("%s", str);
	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
