#include "protocolo.h"

size_t mensagem_obter_tamanho(const Mensagem *mensagem) {
	return (&mensagem->dados - &mensagem->estados) + mensagem->tamanho_dados;
}
