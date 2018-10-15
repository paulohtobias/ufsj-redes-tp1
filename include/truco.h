#ifndef TRUCO_H
#define TRUCO_H

#include "baralho.h"

/* DEFINIÇÕES */
#define NUM_CARTAS 41
#define NUM_JOGADORES 4
#define NUM_CARTAS_MAO 3


/* ENUMS E TIPOS */
typedef enum FASE_JOGO {
	FJ_ENVIANDO_CARTAS,
	FJ_TURNO,
	FJ_PEDIU_TRUCO,
	FJ_EMPATE,
	FJ_FIM_PARTIDA,
	FJ_FIM_JOGO,
	FJ_FIM_QUEDA
} FASE_JOGO;

typedef enum JOGADORES_ATIVOS {
	JA_NINGUEM = 0,
	JA_TIME1 = 5,
	JA_TIME2 = 10,
	JA_TODOS = 0xF
} JOGADORES_ATIVOS;
#define JA_JOGADOR(id) (1 << id)

typedef enum VALOR_PARTIDA {
	VLR_NORMAL = 2,
	VLR_TRUCO = 4,
	VLR_SEIS = 8,
	VLR_NOVE = 10,
	VLR_DOZE = 12
} VALOR_PARTIDA;

typedef struct EstadoJogo {
	uint8_t rodadas[2];  // Melhor de 3
	uint8_t pontos[2];   // [0, 12]
	uint8_t mao_de_10;
	VALOR_PARTIDA valor_partida;
	uint8_t jogos[2];    // 0 ou 1
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


/* VARIÁVEIS GLOBAIS */
extern char cores_times[NUM_JOGADORES][16];
extern char valor_partida_str[5][10];
char pontuacao_str[300];

extern Carta gbaralho[NUM_CARTAS];
extern EstadoJogo gpontuacao;
extern int gvencedor_jogo;
extern int gvencedor_partida;
extern int gvencedor_queda;
extern uint8_t gturno;
extern uint8_t gmao;
extern FASE_JOGO gfase;
extern JOGADORES_ATIVOS gjogadores_ativos;
extern Carta gjogadores_cartas[NUM_JOGADORES][NUM_CARTAS_MAO];
extern int gjogadores_cartas_jogadas[NUM_JOGADORES][NUM_CARTAS_MAO];
extern Carta gcarta_mais_forte;
extern int gjogador_carta_mais_forte;
extern int gempate_parcial;

/* FUNÇÕES */
int terminar_rodada();

int terminar_partida();

int terminar_jogo();

void pontuacao_str_atualizar(EstadoJogo *pontuacao);

#endif //TRUCO_H