#include "truco.h"

/* VARIÁVEIS GLOBAIS */
char cores_times[NUM_JOGADORES][16] = {
	/*        TIME 1         */   /*      TIME 2       */
	"#0000FF", /* Blue       */   "#FF0000", /* Red    */ 
	"#1E90FF", /* DodgerBlue */   "#FF6347"  /* Tomato */
};
char jogador_nome_fmt[60] = "<span font_weight='bold' color='%s'>Jogador %d</span>";
uint8_t valor_partida[5] = {
	VLR_NORMAL,
	VLR_TRUCO,
	VLR_SEIS,
	VLR_NOVE,
	VLR_DOZE
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
	{'7', OUROS, 12, 0},
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
EstadoJogo gestado = {{0, 0}, {0, 0}, {0, 0}, -1, 0, 0, -1, -1, 0, {'-', BARALHO_VIRADO, 0, 0}, -1};
int8_t gvencedor_primeira_rodada = -1;
int8_t gvencedor_partida = -1;
int8_t gvencedor_jogo = -1;
int8_t gvencedor_queda = -1;

JOGADORES_ATIVOS gjogadores_ativos;

EstadoJogador gestado_jogadores[NUM_JOGADORES] = {{0, 0, {'-', BARALHO_VIRADO, 0, 0}}, {0, 0, {'-', BARALHO_VIRADO, 0, 0}}, {0, 0, {'-', BARALHO_VIRADO, 0, 0}}, {0, 0, {'-', BARALHO_VIRADO, 0, 0}}};
Carta gjogadores_cartas[NUM_JOGADORES][NUM_CARTAS_MAO];
int8_t gindice_carta;
uint8_t gjogadores_cartas_jogadas[NUM_JOGADORES][NUM_CARTAS_MAO];

/* FUNÇÕES */
void iniciar_rodada() {
	gturno = 0;
	carta_virar(&gestado.carta_mais_forte);
	gestado.jogador_carta_mais_forte = -1;
	gfase = FJ_TURNO;
}

int8_t terminar_rodada(int8_t vencedor_partida) {

	if (vencedor_partida == -1) {
		//Incrementa a quantidade de rodadas.
		gestado.rodadas[JOGADOR_TIME(gestado.jogador_carta_mais_forte)]++;
	}

	#ifdef DEBUG
	printf("\nterminar_rodada: [%d|%d]\n\n", gestado.rodadas[0], gestado.rodadas[1]);
	#endif //DEBUG

	int8_t i;
	for (i = 0; i < 2; i++) {
		if (vencedor_partida != -1) {
			i = vencedor_partida;
		}
		if (gestado.rodadas[i] == 2 || i == vencedor_partida) {
			int valor = valor_partida[gestado.valor_partida];

			// Se o adversário estiver na mão de 10
			if (gestado.mao_de_10 == !i) {
				valor = VLR_MAO_10;
			}
			
			gestado.pontos[i] += valor;

			//Entrando na mão de 10.
			if (gestado.pontos[i] == 10 && gestado.mao_de_10 == -1) {
				gestado.mao_de_10 = i;

				#if defined DEBUG || defined LOG
				printf("O time %d entrou na mão de 10.\n", i);
				#endif //defined DEBUG || defined LOG
			}

			gestado.jogador_carta_mais_forte = -1;
			gestado.valor_partida = 0;

			return i;
		}
	}
	
	gestado.jogador_atual = gestado.jogador_carta_mais_forte;
	return -1;
}

int8_t terminar_partida() {
	gestado.rodadas[0] = 0;
	gestado.rodadas[1] = 0;
	gestado.mao_de_10 = -1;
	gestado.valor_partida = 0;

	int8_t i;
	for (i = 0; i < 2; i++) {
		if (gestado.pontos[i] >= 12) {
			gestado.jogos[i]++;
			return i;
		}
	}
	
	return -1;
}

int8_t terminar_jogo() {
	int8_t i;
	for (i = 0; i < 2; i++) {
		if (gestado.jogos[i] > 1) {
			return i;
		}
	}
	
	return -1;
}

void pontuacao_str_atualizar() {
	snprintf(pontuacao_str, 512,
		"<b>Partida atual</b>:\n"
			"%s"
			"\t%s (%d pontos)\n"
			"\tEmpate na rodada? %s\n\n"
		
		"<b>Jogos-Pontos-Rodadas</b>:\n"
			"\t<span font_weight='bold' color='%s'>Time 1: %d-%2d-%d</span>\n"
			"\t<span font_weight='bold' color='%s'>Time 2: %d-%2d-%d</span>\n",
		
		gestado.empate ? "\t<span font_weight='bold'>Houve empate. Me mostre sua maior carta.</span>\n" : "",
		valor_partida_str[gestado.valor_partida], valor_partida[gestado.valor_partida],
		gestado.empate_parcial ? "Sim": "Não",
		
		cores_times[0], gestado.jogos[0], gestado.pontos[0], gestado.rodadas[0],
		cores_times[1], gestado.jogos[1], gestado.pontos[1], gestado.rodadas[1]
	);
}

void mesa_str_atualizar(int8_t jogador_id) {
	int i, j;
	char cartas_mao_str[NUM_JOGADORES][40];
	for (i = 0; i < NUM_JOGADORES; i++) {
		memset(cartas_mao_str[i], 0, 40);
		if (i == jogador_id) {
			sprintf(
				cartas_mao_str[i],
				" |%c%s| |%c%s| |%c%s|",
				gjogadores_cartas[i][0].numero, naipe_str[gjogadores_cartas[i][0].naipe],
				gjogadores_cartas[i][1].numero, naipe_str[gjogadores_cartas[i][1].naipe],
				gjogadores_cartas[i][2].numero, naipe_str[gjogadores_cartas[i][2].naipe]
			);
		} else {
			for (j = 0; j < gestado_jogadores[i].qtd_cartas_mao; j++) {
				strcat(cartas_mao_str[i], " |--|");
			}
		}
	}
	snprintf(mesa_str, 512,
		"Jogador 0:%s\n"
			"\tNa mesa: |%c%s|\n\n"
		
		"Jogador 1:%s\n"
			"\tNa mesa: |%c%s|\n\n"
		
		"Jogador 2:%s\n"
			"\tNa mesa: |%c%s|\n\n"
		
		"Jogador 3:%s\n"
			"\tNa mesa: |%c%s|\n\n"
		
		"------------------------------------------------\n"
		"Carta mais alta: %c%s (Jogador %d)",
		
		cartas_mao_str[0], gestado_jogadores[0].carta_jogada.numero, naipe_str[gestado_jogadores[0].carta_jogada.naipe],
		cartas_mao_str[1], gestado_jogadores[1].carta_jogada.numero, naipe_str[gestado_jogadores[1].carta_jogada.naipe],
		cartas_mao_str[2], gestado_jogadores[2].carta_jogada.numero, naipe_str[gestado_jogadores[2].carta_jogada.naipe],
		cartas_mao_str[3], gestado_jogadores[3].carta_jogada.numero, naipe_str[gestado_jogadores[3].carta_jogada.naipe],
		gestado.carta_mais_forte.numero, naipe_str[gestado.carta_mais_forte.naipe], gestado.jogador_carta_mais_forte
	);
}
