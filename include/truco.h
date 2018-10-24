#ifndef TRUCO_H
#define TRUCO_H

#include "baralho.h"

/* DEFINIÇÕES */
#define NUM_CARTAS 41
#define NUM_JOGADORES 4
#define NUM_CARTAS_MAO 3


/* ENUMS E TIPOS */
typedef enum FASE_JOGO {
	FJ_AGUARDANDO_INICIO,
	FJ_ENVIANDO_CARTAS,
	FJ_MAO_DE_10,
	FJ_TURNO,
	FJ_FIM_TURNO,
	FJ_PEDIU_TRUCO,
	FJ_EMPATE,
	FJ_FIM_RODADA,
	FJ_FIM_QUEDA
} FASE_JOGO;

typedef enum JOGADORES_ATIVOS {
	JA_NINGUEM = 0,
	JA_TIME1 = 5,
	JA_TIME2 = 10,
	JA_TODOS = 0xF
} JOGADORES_ATIVOS;
#define JA_JOGADOR(id) (1 << id)
#define JA_TIME(id) (JOGADOR_TIME(id) ? JA_TIME2 : JA_TIME1)

typedef enum VALOR_PARTIDA {
	VLR_NORMAL = 2,
	VLR_TRUCO = 4,
	VLR_SEIS = 8,
	VLR_NOVE = 10,
	VLR_DOZE = 12
} VALOR_PARTIDA;
#define VLR_MAO_10 4

typedef struct EstadoJogo {
	//Pontuação
	uint8_t rodadas[2];  // Melhor de 3
	uint8_t pontos[2];   // [0, 12]
	uint8_t jogos[2];    // 0 ou 1

	int8_t jogador_atual;
	
	uint8_t empate_parcial;  // Se houve empate na rodada até o momento.
	uint8_t empate;          // Se a primeira rodada terminou em empate.
	int8_t time_truco;       // id do último time que pediu truco. -1 se ninguém pediu ainda.
	uint8_t mao_de_10;        // Bitmask. O bit correspondente ao id do time indica se está na mão de 10.
	uint8_t valor_partida;   // índice no vetor global que indica quanto vale a partida.
	Carta carta_mais_forte;
	int8_t jogador_carta_mais_forte;
} EstadoJogo;

typedef struct EstadoJogador {
	int8_t id;
	uint8_t qtd_cartas_mao;
	Carta carta_jogada;
} EstadoJogador;

typedef enum RESPOSTA {
	RSP_NAO,
	RSP_SIM,
	RSP_AUMENTO,
	RSP_INDEFINIDO
} RESPOSTA;

/* VARIÁVEIS GLOBAIS */
extern char cores_times[NUM_JOGADORES][16];
extern char jogador_nome_fmt[60];
extern uint8_t valor_partida[5];
extern char valor_partida_str[5][10];
char pontuacao_str[512];
char mesa_str[512];

extern Carta gbaralho[NUM_CARTAS];
extern EstadoJogo gestado;
int8_t gturno;
int8_t gmao;
int8_t gjogador_baralho;
uint8_t gfase;
extern int8_t gvencedor_primeira_rodada;
extern int8_t gvencedor_partida;
extern int8_t gvencedor_jogo;
extern int8_t gvencedor_queda;

extern JOGADORES_ATIVOS gjogadores_ativos;

extern EstadoJogador gestado_jogadores[NUM_JOGADORES];
extern Carta gjogadores_cartas[NUM_JOGADORES][NUM_CARTAS_MAO];
extern int8_t gindice_carta;
uint8_t carta_no_monte;
extern uint8_t gjogadores_cartas_jogadas[NUM_JOGADORES][NUM_CARTAS_MAO];
uint8_t gresposta[2];


/* FUNÇÕES */
#define JOGADOR_TIME(id) (id % 2)

#define JOGADOR_TIME_ADV(id) (!(id % 2))

#define JOGADOR_ESTA_ATIVO(id) ((gjogadores_ativos & JA_JOGADOR(id)) != 0)

#define MAO_DE_FERRO (gestado.mao_de_10 == 3)

void iniciar_rodada();

int8_t terminar_rodada(int8_t vencedor_partida);

void iniciar_partida();

int8_t terminar_partida();

int8_t terminar_jogo();

void pontuacao_str_atualizar();

void mesa_str_atualizar(int8_t jogador_id);

#endif //TRUCO_H