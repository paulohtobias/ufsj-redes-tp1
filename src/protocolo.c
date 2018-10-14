#include "protocolo.h"

char mensagem_tipo_str[__SMT_QTD][64] = {
	"Deseja queimar sua mão?",
	"Estou te enviando suas cartas.",
	"Seu turno.",
	"Alguém pediu truco|seis|nove|doze.",
	"Houve empate. Me mostre sua maior carta."
	"Fim da partida.",
	"Fim do jogo.",
	"Fim da queda.",
	"Nova mensagem no chat."
};

size_t mensagem_obter_tamanho(const Mensagem *mensagem) {
	return ((void *) mensagem->dados - (void *) &mensagem->estados) + mensagem->tamanho_dados;
}
