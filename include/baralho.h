#ifndef BARALHO_H
#define BARALHO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

typedef enum NAIPE {
	NAIPE_VAZIO,
	BARALHO_VIRADO,
	ESPADAS,
	PAUS,
	COPAS,
	OUROS,
	CORINGA,
	__NAIPE_QTD
} NAIPE;

typedef struct Carta {
	char numero;      // [A|2-7|J|Q|K|*|-|X]
	uint8_t naipe;
	uint8_t poder;    // [0-15]
	uint8_t visivel;  // 0 ou 1
} Carta;

extern char naipe_str[__NAIPE_QTD][10];


void carta_esvaziar(Carta *carta);

void carta_virar(Carta *carta);

void embaralhar(Carta *baralho, int qtd_cartas, int vezes);

#endif //BARALHO_H