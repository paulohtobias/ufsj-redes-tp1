#include "baralho.h"

void embaralhar(Carta *baralho, int qtd_cartas) {
	int i;
	Carta baralho_antigo[qtd_cartas];
	int sorteados[qtd_cartas];
	int posicao;

	Carta troca;

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