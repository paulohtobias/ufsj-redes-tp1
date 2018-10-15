#include "servidor.h"
#include <time.h>

int main(int argc, char *argv[]) {
	int i, j;

	srand((unsigned) time(NULL));

	int chat_socket_fd = criar_socket_servidor();

	pthread_create(&thread_escrita, NULL, t_escrita, NULL);
	
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(chat_socket_fd);

		jogador_init(jogadres + i, i, jsfd);
	}

	//Iniciando o jogo.
	extern pthread_cond_t cond_init;
	pthread_cond_broadcast(&cond_init);

	gvencedor_jogo = -1;
	gmao = rand() % NUM_JOGADORES;
	gjogadores_ativos = JA_JOGADOR(gmao);
	gfase = FJ_ENVIANDO_CARTAS;
	while (gvencedor_jogo == -1) {
		terminar_partida(&gpontuacao);
		embaralhar(gbaralho, NUM_CARTAS);

		//Distribuindo as cartas.
		pthread_mutex_lock(&mutex_jogo);
		for (i = 0; i < NUM_JOGADORES; i++) {
			for (j = 0; j < 3; j++) {
				gjogadores_cartas[i][j] = gbaralho[i * NUM_JOGADORES + j];
			}
			pthread_mutex_lock(&mutex_broadcast);
			mensagem_enviando_cartas(&gmensagem, gjogadores_cartas[i]);
			new_msg = MSG_JOGADOR(i);
			pthread_cond_signal(&cond_new_msg);
			pthread_mutex_unlock(&mutex_broadcast);
		}
		gfase = FJ_TURNO;
		pthread_mutex_unlock(&mutex_jogo);

		gvencedor_partida = -1;
		while (gvencedor_partida == -1) {
		}

		pthread_mutex_lock(&mutex_jogo);
		gmao = (gmao + 1) % NUM_JOGADORES;
		pthread_mutex_unlock(&mutex_jogo);
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadres[i].thread.leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}