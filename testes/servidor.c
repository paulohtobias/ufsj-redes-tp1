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
#define BACKLOG 4

#define BUFF_SIZE 1024

int client_count = 0;
int client_sockets[BACKLOG] = {0};
int chat_socket_fd;

pthread_t *threads;
pthread_mutex_t mutex_broadcast = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *mutexes;
pthread_cond_t *conds;
int *new_mgs;
size_t *msg_len;

char **buff_copia;

int s_accept(int ssfd);

void *thread_escrita(void *);

void *thread_leitura(void *);

int main(int argc, char *argv[]) {
	int retval;

	int i;
	char buff[BUFF_SIZE];
	
	chat_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (chat_socket_fd == -1) {
		handle_error(chat_socket_fd, "socket");
	}

	int enable = 1;
	retval = setsockopt(chat_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if (retval < 0) {
		handle_error(retval, "setsockopt(SO_REUSEADDR) failed");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	retval = bind(chat_socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (retval == -1) {
		handle_error(retval, "bind");
	}

	retval = listen(chat_socket_fd, BACKLOG);
	if (retval == -1) {
		handle_error(retval, "listen");
	}

	//Calcula a quantidade de clientes e threads necessárias.
	int expected_clients = atoi(argv[1]);
	int threads_len = expected_clients * 2;
	
	//Aloca os vetores de buffer.
	buff_copia = malloc(expected_clients);
	
	//Aloca memória para as threads, mutexes e conds.
	threads = malloc(threads_len * sizeof(pthread_t));
	mutexes = malloc(expected_clients * sizeof(pthread_mutex_t));
	conds = malloc(expected_clients * sizeof(pthread_cond_t));
	new_mgs = malloc(expected_clients * sizeof(int));
	msg_len = malloc(expected_clients * sizeof(size_t));
	
	for (i = 0; i < expected_clients; i++) {
		//Faz conexão inicial com o cliente.
		s_accept(chat_socket_fd);

		//Calcula o índice do cliente.
		int *index = malloc(sizeof(int));
		*index = client_count - 1;
		
		//Inicialização.
		buff_copia[*index] = malloc(BUFF_SIZE);
		pthread_mutex_init(&mutexes[*index], NULL);
		pthread_cond_init(&conds[*index], NULL);
		new_mgs[*index] = 0;
		msg_len[*index] = 0;
		
		//Inicia as threads de leitura e escrita.
		pthread_create(threads + i, NULL, thread_leitura, index);
		pthread_create(threads + i + expected_clients, NULL, thread_escrita, index);
	}

	for (i = 0; i <= expected_clients; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

void *thread_escrita(void *arg){
	int client_socket_index = *((int *)arg);
	free(arg);

	int i;
	while (1) {
		//Espera uma nova mensagem chegar.
		pthread_mutex_lock(&mutexes[client_socket_index]);
		while (!new_mgs[client_socket_index]) {
			pthread_cond_wait(&conds[client_socket_index], &mutexes[client_socket_index]);
		}
		pthread_mutex_unlock(&mutexes[client_socket_index]);

		//Envia a mensagem para os clientes
		pthread_mutex_lock(&mutex_broadcast);
		for (i = 0; i < client_count; i++) {
			write(client_sockets[i], buff_copia[client_socket_index], msg_len[client_socket_index]);
		}
		new_mgs[client_socket_index] = 0;
		pthread_mutex_unlock(&mutex_broadcast);
	}
}

void *thread_leitura(void *arg){
	int client_socket_index = *((int *)arg);
	int i, j, retval;
	char buff[BUFF_SIZE];
	char client_name[50];

	sprintf(client_name, "\033[0;3%dmclient %d: \033[0m", client_socket_index + 1, client_socket_index);

	while (1) {
		retval = read(client_sockets[client_socket_index], buff, BUFF_SIZE);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		}
		
		#ifdef DEBUG
		printf("[cliente %d] <%d> '%s'\n", client_socket_index, retval, buff);
		#endif
		
		//Copia a msg do cliente p/ buff_copia.
		snprintf(buff_copia[client_socket_index], BUFF_SIZE, "%s%s", client_name, buff);
		msg_len[client_socket_index] = strlen(buff_copia[client_socket_index]) + 1;

		#ifdef DEBUG
		printf("[cliente %d] <%d> '%s'\n", client_socket_index, msg_len[client_socket_index], buff_copia[client_socket_index]);
		#endif
		
		//Sinaliza p/ thread de escrita para enviar a msg aos clientes.
		new_mgs[client_socket_index] = 1;
		pthread_cond_signal(&conds[client_socket_index]);
	}
}

int s_accept(int ssfd) {
	int retval = 0;

	int client_socket_fd;
	socklen_t client_len;
	struct sockaddr_in client_addr;

	client_len = sizeof(client_addr);

	client_socket_fd = accept(ssfd, (struct sockaddr *) &client_addr, &client_len);

	if (client_socket_fd == -1) {
		handle_error(client_socket_fd, "accept");
	}

	client_sockets[client_count++] = client_socket_fd;

	printf("[Server]: client %d connected.\n", client_socket_fd);

	return client_socket_fd;
}
