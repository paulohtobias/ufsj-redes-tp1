#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "conexao.h"

/* TIPOS */
typedef struct ThreadJogador {
	pthread_t leitura;
	pthread_t escrita;

	pthread_mutex_t new_msg_mutex;
	pthread_cond_t new_msg_cond;
	uint8_t new_msg;
} ThreadJogador;

typedef struct Jogador {
	int8_t id;			// [0-4]
	int socket_fd;

	EstadoJogador estado;

	ThreadJogador thread;
} Jogador;


/* VARIÁVEIS GLOBAIS*/
pthread_t thread_escrita;
extern pthread_mutex_t mutex_jogo;
extern pthread_mutex_t mutex_broadcast;
extern pthread_mutex_t mutex_new_msg;
extern pthread_cond_t cond_new_msg;
extern uint8_t new_msg;
Mensagem gmensagem;
Jogador jogadres[NUM_JOGADORES];


/* FUNÇÕES */
int criar_socket_servidor();

int s_accept(int ssfd);

void jogador_init(Jogador *jogador, uint8_t id, int sfd);

void *t_leitura(void *args);

void *t_escrita(void *args);

#endif //SERVIDOR_H
