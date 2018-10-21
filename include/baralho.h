#ifndef BARALHO_H
#define BARALHO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

typedef enum NAIPE {
	BARALHO_VIRADO,
	ESPADAS,
	PAUS,
	COPAS,
	OUROS,
	CORINGA
} NAIPE;

typedef struct Carta {
	char numero;      // [A|2-7|J|Q|K|*|-]
	uint8_t naipe;
	uint8_t poder;    // [0-15]
	uint8_t visivel;  // 0 ou 1
} Carta;

extern char naipe_str[6][10];

void carta_virar(Carta *carta);

void embaralhar(Carta *baralho, int qtd_cartas);

#endif //BARALHO_H