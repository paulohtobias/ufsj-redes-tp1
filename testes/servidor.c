#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define handle_error(cod, msg)\
	perror(msg); exit(cod);

#define PORT 2222
#define BACKLOG 4

#define BUFF_SIZE 1024

int qc = 0;
int css[BACKLOG] = {0};

int s_accept(int ssfd);

int main(int argc, char *argv[]) {
	int retval;
	int sfd;

	int i;
	char buff[BUFF_SIZE];
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		handle_error(sfd, "socket");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	retval = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (retval == -1) {
		handle_error(retval, "bind");
	}

	retval = listen(sfd, BACKLOG);
	if (retval == -1) {
		handle_error(retval, "listen");
	}

	int expected_clients = atoi(argv[1]);
	for (i = 0; i < expected_clients; i++) {
		s_accept(sfd);
	}

	while (1) {
		for (i = 0; i < qc; i++) {
			retval = read(css[i], buff, BUFF_SIZE);
			if (retval == -1) {
				handle_error(retval, "read");
			}
			
			printf("[Servidor] Mensagem do cliente %d: '%s'\n", i, buff);
		}
	}

	return 0;
}

int s_accept(int ssfd) {
	int retval = 0;

	int csfd;
	socklen_t client_len;
	struct sockaddr_in client_addr;

	client_len = sizeof(client_addr);

	csfd = accept(ssfd, (struct sockaddr *) &client_addr, &client_len);

	if (csfd == -1) {
		handle_error(csfd, "accept");
	}

	css[qc++] = csfd;

	printf("Servidor: cliente %d conectado.\n", csfd);

	//close(csfd);

	return retval;
}
