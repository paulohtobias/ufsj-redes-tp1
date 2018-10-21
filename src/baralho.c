#include "baralho.h"

char naipe_str[6][10] = {
	"-",
	"♠",
	"♣",
	"♥",
	"♦",
	"*"
};

void carta_virar(Carta *carta) {
	carta->numero = '-';
	carta->naipe = BARALHO_VIRADO;
	carta->poder = 0;
	carta->visivel = 0;
}

void embaralhar(Carta *baralho, int qtd_cartas) {
	int i;
	Carta baralho_antigo[qtd_cartas];
	int sorteados[qtd_cartas];
	int posicao;

	//Inicialização
	for (i = 0; i < qtd_cartas; i++) {
		sorteados[i] = 0;
		baralho_antigo[i] = baralho[i];
	}

	for (i = 0; i < qtd_cartas; i++) {
		//Sorteia a nova posição.
		do {
			posicao = rand() % qtd_cartas;
		} while (sorteados[posicao] == 1);
		sorteados[posicao] = 1;

		baralho[posicao] = baralho_antigo[i];
	}
}