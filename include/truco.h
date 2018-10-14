#ifndef TRUCO_H
#define TRUCO_H

#include "baralho.h"

/* DEFINIÇÕES E VARIÁVEIS GLOBAIS */
#define NUM_JOGADORES 4
extern char cores_times[NUM_JOGADORES][16];

typedef enum VALOR_PARTIDA {
	VLR_NORMAL = 2,
	VLR_TRUCO = 4,
	VLR_SEIS = 8,
	VLR_NOVE = 10,
	VLR_DOZE = 12
} VALOR_PARTIDA;

typedef struct EstadoJogo {
	uint8_t pontos[2];
	VALOR_PARTIDA valor_partida;
	uint8_t jogos[2]; //0 ou 1
} EstadoJogo;

typedef struct EstadoJogador {
	uint8_t id;
	uint8_t qtd_cartas_mao;
	Carta carta_jogada;
} EstadoJogador;

typedef enum RESPOSTAS {
	RSP_NAO,
	RSP_SIM,
	RSP_AUMENTO
} RESPOSTAS;

#endif //TRUCO_H