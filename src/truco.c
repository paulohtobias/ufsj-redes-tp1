#include "truco.h"

/* VARIÁVEIS GLOBAIS */
char cores_times[NUM_JOGADORES][16] = {
	/*        TIME 1         */   /*      TIME 2       */
	"#0000FF", /* Blue       */   "#FF0000", /* Red    */ 
	"#1E90FF", /* DodgerBlue */   "#FF6347"  /* Tomato */
};
char valor_partida_str[5][10] = {
	"Normal",
	"TRUCO",
	"SEIS",
	"NOVE",
	"DOZE"
};
Carta gbaralho[NUM_CARTAS] = {
	{'4', PAUS, 15, 0},
	{'7', COPAS, 14, 0},
	{'A', ESPADAS, 13, 0},
	{'7', COPAS, 12, 0},
	{'*', CORINGA, 11, 0},
	
	{'3', ESPADAS, 10, 0},
	{'3', PAUS, 10, 0},
	{'3', COPAS, 10, 0},
	{'3', OUROS, 10, 0},

	{'2', ESPADAS, 9, 0},
	{'2', PAUS, 9, 0},
	{'2', COPAS, 9, 0},
	{'2', OUROS, 9, 0},

	{'A', PAUS, 8, 0},
	{'A', COPAS, 8, 0},
	{'A', OUROS, 8, 0},

	{'K', ESPADAS, 7, 0},
	{'K', PAUS, 7, 0},
	{'K', COPAS, 7, 0},
	{'K', OUROS, 7, 0},

	{'J', ESPADAS, 6, 0},
	{'J', PAUS, 6, 0},
	{'J', COPAS, 6, 0},
	{'J', OUROS, 6, 0},

	{'Q', ESPADAS, 5, 0},
	{'Q', PAUS, 5, 0},
	{'Q', COPAS, 5, 0},
	{'Q', OUROS, 5, 0},

	{'7', ESPADAS, 4, 0},
	{'7', PAUS, 4, 0},

	{'6', ESPADAS, 3, 0},
	{'6', PAUS, 3, 0},
	{'6', COPAS, 3, 0},
	{'6', OUROS, 3, 0},

	{'5', ESPADAS, 2, 0},
	{'5', PAUS, 2, 0},
	{'5', COPAS, 2, 0},
	{'5', OUROS, 2, 0},

	{'4', ESPADAS, 1, 0},
	{'4', COPAS, 1, 0},
	{'4', OUROS, 1, 0}
};
EstadoJogo gpontuacao = {{0, 0}, {0, 0}, 0, VLR_NORMAL, {0, 0}};
int8_t gvencedor_partida = -1;
int8_t gvencedor_jogo = -1;
int8_t gvencedor_queda = -1;
int8_t gturno = -1;
int8_t gmao = -1;
FASE_JOGO gfase = FJ_AGUARDANDO_INICIO;
JOGADORES_ATIVOS gjogadores_ativos;
Carta gjogadores_cartas[NUM_JOGADORES][NUM_CARTAS_MAO];
int gjogadores_cartas_jogadas[NUM_JOGADORES][NUM_CARTAS_MAO];
Carta gcarta_mais_forte = {'-', BARALHO_VIRADO, 0, 0};
int8_t gjogador_carta_mais_forte = -1;
int gempate_parcial = 0;

/* FUNÇÕES */
int terminar_rodada() {
	gturno = 0;
	gempate_parcial = 0;
	carta_virar(&gcarta_mais_forte);
	gpontuacao.rodadas[gjogador_carta_mais_forte]++;
	gjogador_carta_mais_forte = -1;
	
	int i;
	for (i = 0; i < 2; i++) {
		if (gpontuacao.rodadas[i] == 2) {
			int valor = gpontuacao.valor_partida;

			// Se o adversário estiver na mão de 10
			if (gpontuacao.mao_de_10 == !i) {
				valor = 4;
			}
			
			gpontuacao.pontos[i] += valor;

			if (gpontuacao.valor_partida == 10 && gpontuacao.mao_de_10 == -1) {
				gpontuacao.mao_de_10 = i;
			}

			return i;
		}
	}
	
	return -1;
}

int terminar_partida() {
	gpontuacao.rodadas[0] = 0;
	gpontuacao.rodadas[1] = 0;
	gpontuacao.mao_de_10 = -1;
	gpontuacao.valor_partida = VLR_NORMAL;

	int i;
	for (i = 0; i < 2; i++) {
		if (gpontuacao.pontos[i] > 12) {
			gpontuacao.jogos[i]++;
			return i;
		}
	}
	
	return -1;
}

int terminar_jogo() {
	int i;
	for (i = 0; i < 2; i++) {
		if (gpontuacao.jogos[i] > 1) {
			return i;
		}
	}
	
	return -1;
}

void pontuacao_str_atualizar(EstadoJogo *pontuacao) {
	snprintf(pontuacao_str, 300,
		"<b>Partida atual</b>: %s (%d pontos)\n"
		"<b>Jogos-Pontos</b>:\n"
			"\t<span font_weight='bold' color='%s'>Time 1: %d-%02d</span>\n"
			"\t<span font_weight='bold' color='%s'>Time 2: %d-%02d</span>\n",
		valor_partida_str[pontuacao->valor_partida], pontuacao->valor_partida,
		cores_times[0], pontuacao->jogos[0], pontuacao->pontos[0],
		cores_times[1], pontuacao->jogos[1], pontuacao->pontos[1]
	);
}
