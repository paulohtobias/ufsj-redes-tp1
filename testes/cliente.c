#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define handle_error(cod, msg)\
	perror(msg); exit(cod);

#define PORT 2222

#define BUFF_SIZE 1024

int ssfd;
pthread_t threads[2];

///Lê o log do chat do servidor e exibe na tela.
void *thread_leitura(void *);

///Envia as mensagens do cliente ao servidor.
void *thread_escrita(void *);

int main(int argc, char *argv[]) {
	int i;
	int retval;

	char buff[BUFF_SIZE];
	
	ssfd = socket(AF_INET, SOCK_STREAM, 0);
	if (ssfd == -1) {
		handle_error(ssfd, "socket");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	retval = connect(ssfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if (retval == -1) {
		handle_error(retval, "connect");
	}

	printf("[Client] connected to server.\n");

	pthread_create(threads, NULL, thread_escrita, NULL);
	pthread_create(threads + 1, NULL, thread_leitura, NULL);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	return 0;
}

void *thread_leitura(void *arg){
	int retval;
	char buff[BUFF_SIZE];
	while (1) {
		retval = read(ssfd, buff, BUFF_SIZE);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		}

		if (retval > 0) {
			puts(buff);
		}
	}
}

void *thread_escrita(void *arg){
	int len;
	char buff[BUFF_SIZE];
	while (1) {
		//scanf("%s", buff);
		len = read(0, buff, BUFF_SIZE);
		if (len == -1) {
			handle_error(1, "thread_escrita-read");
		}
		buff[len - 1] = '\0';
		if (write(ssfd, buff, len) == -1) {
			handle_error(1, "thread_escrita-write");
		}
	}
}
