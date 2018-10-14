#include "servidor.h"

int main(int argc, char *argv[]) {
	int i;

	int chat_socket_fd = servidor_init();

	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(chat_socket_fd);

		jogador_init(jogadres + i, i, jsfd);
	}

	pthread_create(&thread_escrita, NULL, t_escrita, NULL);

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadres[i].thread.leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}