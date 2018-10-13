#ifndef BARALHO_H
#define BARALHO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum NAIPE {
	ESPADAS,
	PAUS,
	COPAS,
	OUROS,
	CORINGA,
} NAIPE;

typedef struct Carta {
	NAIPE naipe;
	char valor; // [A|2-7|J|Q|K|*]
	uint8_t poder; // [0-14]
	uint8_t visivel;
} Carta;

void embaralhar(Carta *baralho, int qtd_cartas);

#endif //BARALHO_H