#include "baralho.h"

char naipe_str[__NAIPE_QTD][10] = {
	"X",
	"-",
	"♠",
	"♣",
	"♥",
	"♦",
	"*"
};

void carta_esvaziar(Carta *carta) {
	carta->numero = 'X';
	carta->naipe = NAIPE_VAZIO;
	carta->poder = 0;
	carta->visivel = 0;
}

void carta_virar(Carta *carta) {
	carta->numero = '-';
	carta->naipe = BARALHO_VIRADO;
	carta->poder = 0;
	carta->visivel = 0;
}

void embaralhar(Carta *baralho, int qtd_cartas, int vezes) {
	int i, j;
	Carta baralho_antigo[qtd_cartas];
	int sorteados[qtd_cartas];
	int posicao;

	for (i = 0; i < vezes; i++) {
		//Inicialização
		for (j = 0; j < qtd_cartas; j++) {
			sorteados[j] = 0;
			baralho_antigo[j] = baralho[j];
		}

		for (j = 0; j < qtd_cartas; j++) {
			//Sorteia a nova posição.
			do {
				posicao = rand() % qtd_cartas;
			} while (sorteados[posicao] == 1);
			sorteados[posicao] = 1;

			baralho[posicao] = baralho_antigo[j];
		}
	}
}