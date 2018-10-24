#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "conexao.h"

/* TIPOS */
typedef struct Jogador {
	int8_t id;			// [0-4]
	int socket_fd;

	pthread_t thread_leitura;
} Jogador;


/* VARIÁVEIS GLOBAIS*/
Mensagem gmensagem;
Jogador jogadores[NUM_JOGADORES];

pthread_t thread_escrita;
extern pthread_mutex_t mutex_jogo;
extern pthread_cond_t cond_jogo;

extern pthread_mutex_t mutex_init;
extern pthread_cond_t cond_init;
extern int num_jogadores;

extern pthread_mutex_t mutex_broadcast;
extern pthread_mutex_t mutex_new_msg;
extern pthread_cond_t cond_new_msg;
extern uint8_t new_msg;


/* FUNÇÕES */
int criar_socket_servidor();

int s_accept(int ssfd);

void jogador_init(Jogador *jogador, uint8_t id, int sfd);

void *t_leitura(void *args);

int avisar_truco(int8_t jogador_id);

void enviar_mensagem(const Mensagem *mensagem, uint8_t new_msg);

void esperar_resposta(RESPOSTA resposta_maxima);


void servidor_mensagem_bem_vindo(int8_t id);

void servidor_mensagem_processando(uint8_t remetentes, const char *texto);

void servidor_mensagem_atualizar_estado(const char *texto);

void servidor_mensagem_enviar_cartas(int8_t id);

void servidor_mensagem_seu_turno();

void servidor_mensagem_aguardar_turno();

void servidor_mensagem_jogada_aceita();

void servidor_mensagem_truco(int8_t id);

void servidor_mensagem_empate();

void servidor_mensagem_mao_de_10(uint8_t time);

void servidor_mensagem_fim_queda();

void servidor_mensagem_chat(const char *texto, uint8_t tamanho_dados);

#endif //SERVIDOR_H
