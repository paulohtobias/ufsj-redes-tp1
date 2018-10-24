#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#include "truco.h"

/* DEFINIÇÕES */
#define BUFF_SIZE 256
#define handle_error(cod, msg)\
	perror(msg); exit(cod);


/* ENUMS E TIPOS */
//todo: smt_jogada_aceita
typedef enum MENSAGEM_TIPO {
	SMT_ERRO,
	SMT_BEM_VINDO,
	SMT_PROCESSANDO,
	SMT_ENVIANDO_CARTAS,
	SMT_SEU_TURNO,
	SMT_AGUARDANDO_TURNO,
	SMT_JOGADA_ACEITA,
	SMT_TRUCO,
	SMT_EMPATE,
	SMT_MAO_DE_10,
	SMT_FIM_QUEDA,
	SMT_CHAT,
	__SMT_QTD
} MENSAGEM_TIPO;

extern char mensagem_tipo_str[__SMT_QTD][BUFF_SIZE];

enum {
	MSG_NINGUEM = 0,
	MSG_TIME1 = 5,
	MSG_TIME2 = 10,
	MSG_TODOS = 0xF
};
#define MSG_JOGADOR(id) (1 << id)
#define MSG_TIME(id) (JOGADOR_TIME(id) ? MSG_TIME2 : MSG_TIME1)
#define MSG_TIME_ADV(id) (JOGADOR_TIME(id) ? MSG_TIME1 : MSG_TIME2)

typedef struct Mensagem {
	/* METADADOS */
	uint8_t atualizar_estado_jogo;         // Se é pra atualizar o estado do jogo.
	uint8_t atualizar_estado_jogadores;    // Se é pra atualizar os estados dos jogadores.
	uint8_t tipo;                          // Tipo da mensagem.
	uint8_t tamanho_dados;                 // Tamanho dos dados da mensagem.

	/* DADOS */
	EstadoJogo estado_jogo;
	EstadoJogador estado_jogadores[NUM_JOGADORES];
	uint8_t dados[BUFF_SIZE];
} Mensagem;


/* FUNÇÕES */
int mensagem_receber(int sfd, Mensagem *mensagem);

int mensagem_enviar(const Mensagem *mensagem, int sfd);


size_t mensagem_obter_tamanho(const Mensagem *mensagem);

void mensagem_definir(Mensagem *mensagem, MENSAGEM_TIPO tipo, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES], const void *dados, uint8_t tamanho_dados);

void mensagem_simples(Mensagem *mensagem, MENSAGEM_TIPO tipo);

void mensagem_atualizar_estado(Mensagem *mensagem, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES]);

void mensagem_somente_dados(Mensagem *mensagem, MENSAGEM_TIPO tipo, const void *dados, uint8_t tamanho_dados);


void mensagem_obter_id(const Mensagem *mensagem, int8_t *id);

void mensagem_bem_vindo(Mensagem *mensagem, int8_t id);

void mensagem_obter_estado_jogo(const Mensagem *mensagem, EstadoJogo *estado_jogo);

void mensagem_obter_estado_jogadores(const Mensagem *mensagem, EstadoJogador estado_jogadores[NUM_JOGADORES]);

void mensagem_definir_cartas(Mensagem *mensagem, const Carta cartas[NUM_CARTAS_MAO]);

void mensagem_obter_cartas(const Mensagem *mensagem, Carta cartas[NUM_CARTAS_MAO]);

void mensagem_obter_carta(const Mensagem *mensagem, int8_t *indice_carta);

void mensagem_definir_carta(Mensagem *mensagem, int8_t indice_carta);

void mensagem_aguardar_turno(Mensagem *mensagem, int8_t id);

void mensagem_truco(Mensagem *mensagem, int8_t id);

void mensagem_obter_resposta(const Mensagem *mensagem, uint8_t *resposta);

void mensagem_definir_resposta(Mensagem *mensagem, RESPOSTA resposta);

char *mensagem_obter_texto(const Mensagem *mensagem, char *texto);

void mensagem_definir_textof(Mensagem *mensagem, const char *format, ...);


void mensagem_processando(Mensagem *mensagem, const char *texto);

void mensagem_seu_turno(Mensagem *mensagem);

void mensagem_jogada_aceita(Mensagem *mensagem, const EstadoJogo *estado_jogo, int8_t indice_carta);

void mensagem_empate(Mensagem *mensagem);

void mensagem_mao_de_10(Mensagem *mensagem);

void mensagem_fim_queda(Mensagem *mensagem, const EstadoJogo *estado, uint8_t time_vencedor);

void mensagem_chat(Mensagem *mensagem, const char *texto, uint8_t tamanho_texto);


void mensagem_print(const Mensagem *mensagem, const char *titulo);

#endif //PROTOCOLO_H
