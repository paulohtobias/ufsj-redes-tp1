#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "protocolo.h"


/* TIPOS */
typedef struct ThreadJogador {
	pthread_t leitura;
	pthread_t escrita;

	pthread_mutex_t new_msg_mutex;
	pthread_cond_t new_msg_cond;
	uint8_t new_msg;
} ThreadJogador;

typedef struct Jogador {
	uint8_t id;			// [0-4]
	int socket_fd;

	ThreadJogador thread;
} Jogador;


/* VARIÁVEIS GLOBAIS*/
pthread_t thread_escrita;
pthread_mutex_t mutex_broadcast;
pthread_mutex_t mutex_new_msg;
pthread_cond_t cond_new_msg;
uint8_t new_msg;
Mensagem gmensagem;
Jogador jogadres[NUM_JOGADORES];


/* FUNÇÕES */
void jogador_init(Jogador *jogador, uint8_t id, int sfd);

void *t_leitura(void *args);

void *t_escrita(void *args);

#endif //SERVIDOR_H
