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
typedef enum MENSAGEM_TIPO {
	SMT_ERRO,
	SMT_BEM_VINDO,
	SMT_PROCESSANDO,
	SMT_ENVIANDO_CARTAS,
	SMT_SEU_TURNO,
	SMT_TRUCO,
	SMT_EMPATE,
	SMT_FIM_RODADA,
	SMT_FIM_PARTIDA,
	SMT_FIM_JOGO,
	SMT_FIM_QUEDA,
	SMT_CHAT,
	__SMT_QTD
} MENSAGEM_TIPO;

extern char mensagem_tipo_str[__SMT_QTD][64];

enum {
	MSG_NINGUEM = 0,
	MSG_TIME1 = 5,
	MSG_TIME2 = 10,
	MSG_TODOS = 0xF
};
#define MSG_JOGADOR(id) (1 << id)

typedef struct Mensagem {
	uint8_t estados;                // Quantos estados serão atualizados.
	uint8_t atualizar_pontuacao;    // Se é pra atualizar a pontuação.
	MENSAGEM_TIPO tipo;             // Tipo da mensagem.
	uint8_t tamanho_dados;          // Tamanho dos dados da mensagem.
	uint8_t dados[BUFF_SIZE];       // Dados.
} Mensagem;


/* FUNÇÕES */
int mensagem_enviar(const Mensagem *mensagem, int sfd);

size_t mensagem_obter_tamanho(const Mensagem *mensagem);

void mensagem_definir(Mensagem *mensagem, MENSAGEM_TIPO tipo, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo, uint8_t *dados, uint8_t tamanho_dados);

void mensagem_simples(Mensagem *mensagem, MENSAGEM_TIPO tipo);

void mensagem_atualizar_estado(Mensagem *mensagem, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo);

void mensagem_bem_vindo(Mensagem *mensagem);

void mensagem_processando(Mensagem *mensagem, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo, char *texto, uint8_t tamanho_texto);

void mensagem_enviando_cartas(Mensagem *mensagem, const Carta cartas[NUM_CARTAS_MAO]);

void mensagem_seu_turno(Mensagem *mensagem);

void mensagem_truco(Mensagem *mensagem);

void mensagem_empate(Mensagem *mensagem);

//todo: passar o time vencedor
void mensagem_fim_rodada(Mensagem *mensagem);

//todo: passar o time vencedor
void mensagem_fim_partida(Mensagem *mensagem);

//todo: passar o time vencedor
void mensagem_fim_jogo(Mensagem *mensagem);

//todo: passar o time vencedor
void mensagem_fim_queda(Mensagem *mensagem);

void mensagem_chat(Mensagem *mensagem, char *texto, uint8_t tamanho_texto);

void mensagem_definir_textof(Mensagem *mensagem, const char *format, ...);

void mensagem_print(const Mensagem *mensagem, const char *titulo);

void mensagem_obter_pontuacao(const Mensagem *mensagem, EstadoJogo *pontuacao);

void mensagem_obter_estado_jogadores(const Mensagem *mensagem, EstadoJogador estado_jogadores[NUM_JOGADORES]);

void mensagem_obter_carta(const Mensagem *mensagem, int *indice_carta);

void mensagem_obter_cartas(const Mensagem *mensagem, Carta cartas[NUM_CARTAS_MAO]);

char *mensagem_obter_texto(const Mensagem *mensagem);

#endif //PROTOCOLO_H
