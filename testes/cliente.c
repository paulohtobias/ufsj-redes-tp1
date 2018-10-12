#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define handle_error(cod, msg)\
	perror(msg); exit(cod);

#define PORT 2222

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
	int retval;
	int sfd;

	char buff[BUFF_SIZE];
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		handle_error(sfd, "socket");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	retval = connect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if (retval == -1) {
		handle_error(retval, "connect");
	}

	printf("Client %d: connected to server.\n", sfd);

	int i = 0;
	int st = atoi(argv[2]);
	while (1) {
		snprintf(buff, BUFF_SIZE, "carta %s-%02d", argv[1], i++);
		write(sfd, buff, strlen(buff) + 1);
		sleep(st);
	}

	return 0;
}
