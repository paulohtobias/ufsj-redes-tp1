#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>

#include "truco.h"

/* DEFINIÇÕES */
#define BUFF_SIZE 256
#define handle_error(cod, msg)\
	perror(msg); exit(cod);


/* ENUMS E TIPOS */
typedef enum MENSAGEM_TIPO {
	SMT_QUER_QUEIMAR_MAO,
	SMT_ENVIANDO_CARTAS,
	SMT_SEU_TURNO,
	SMT_TRUCO,
	SMT_EMPATE,
	SMT_FIM_PARTIDA,
	SMT_FIM_JOGO,
	SMT_FIM_QUEDA,
	SMT_CHAT,
	__SMT_QTD
} MENSAGEM_TIPO;

char mensagem_tipo_str[__SMT_QTD][64] = {
	"Deseja queimar sua mão?",
	"Estou te enviando suas cartas.",
	"Seu turno.",
	"Alguém pediu truco|seis|nove|doze.",
	"Houve empate. Me mostre sua maior carta."
	"Fim da partida.",
	"Fim do jogo.",
	"Fim da queda.",
	"Nova mensagem no chat."
};

enum {
	MSG_NINGUEM = 0,
	MSG_TIME1 = 5,
	MSG_TIME2 = 10,
	MSG_TODOS = 0xF
};

typedef struct Mensagem {
	uint8_t estados;			// Bitmask p/ cada jogador.
	uint8_t pontuacao;			// Se é pra atualizar a pontuação.
	MENSAGEM_TIPO tipo;			// Tipo da mensagem.
	uint8_t tamanho_dados;		// Tamanho dos dados da mensagem.
	uint8_t dados[BUFF_SIZE];	// Dados.
} Mensagem;


/* FUNÇÕES */
size_t mensagem_obter_tamanho(const Mensagem *mensagem);

#endif //PROTOCOLO_H
