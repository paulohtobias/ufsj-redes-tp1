#include "protocolo.h"

char mensagem_tipo_str[__SMT_QTD][64] = {
	"Houve um erro no sistema.",
	"Bem vindo ao truco!",
	"Processando.",
	"Estou te enviando suas cartas.",
	"Seu turno.",
	"Alguém pediu truco|seis|nove|doze.",
	"Houve empate. Me mostre sua maior carta.",
	"Fim da rodada.",
	"Fim da partida.",
	"Fim do jogo.",
	"Fim da queda.",
	"Nova mensagem no chat."
};

int mensagem_enviar(const Mensagem *mensagem, int sfd) {
	return write(sfd, mensagem, mensagem_obter_tamanho(mensagem));
}

size_t mensagem_obter_tamanho(const Mensagem *mensagem) {
	return ((void *) mensagem->dados - (void *) &mensagem->estados) + mensagem->tamanho_dados;
}

void mensagem_definir(Mensagem *mensagem, MENSAGEM_TIPO tipo, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo, uint8_t *dados, uint8_t tamanho_dados) {
	mensagem->tipo = tipo;
	mensagem->estados = qtd_estados;
	mensagem->atualizar_pontuacao = atualizar_pontuacao;

	mensagem->tamanho_dados = 0;
	memset(mensagem->dados, 0, BUFF_SIZE);
	memcpy(mensagem->dados, estado_jogadores, mensagem->estados * sizeof(EstadoJogador));
	mensagem->tamanho_dados = mensagem->estados * sizeof(EstadoJogador);
	if (atualizar_pontuacao) {
		memcpy(&mensagem->dados[mensagem->tamanho_dados], estado_jogo, sizeof(EstadoJogo));
		mensagem->tamanho_dados += sizeof(EstadoJogo);
	}
	memcpy(&mensagem->dados[mensagem->tamanho_dados], &dados, tamanho_dados);
	mensagem->tamanho_dados += tamanho_dados;
}

void mensagem_simples(Mensagem *mensagem, MENSAGEM_TIPO tipo) {
	mensagem_definir(mensagem, tipo, 0, NULL, 0, NULL, NULL, 0);
}

void mensagem_atualizar_estado(Mensagem *mensagem, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo) {
	mensagem_definir(mensagem, SMT_PROCESSANDO, qtd_estados, estado_jogadores, 1, estado_jogo, NULL, 0);
}

void mensagem_bem_vindo(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_BEM_VINDO);
}

void mensagem_processando(Mensagem *mensagem, uint8_t qtd_estados, const EstadoJogador *estado_jogadores, uint8_t atualizar_pontuacao, const EstadoJogo *estado_jogo, char *texto, uint8_t tamanho_texto) {
	mensagem_definir(mensagem, SMT_PROCESSANDO, qtd_estados, estado_jogadores, atualizar_pontuacao, estado_jogo, (uint8_t *) texto, tamanho_texto);
}

void mensagem_enviando_cartas(Mensagem *mensagem, const Carta cartas[NUM_CARTAS_MAO]) {
	mensagem_definir(mensagem, SMT_ENVIANDO_CARTAS, 0, NULL, 0, NULL, (uint8_t *) cartas, NUM_CARTAS_MAO * sizeof(Carta));
}

void mensagem_seu_turno(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_SEU_TURNO);
}

void mensagem_truco(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_TRUCO);
}

void mensagem_empate(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_EMPATE);
}

void mensagem_fim_rodada(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_FIM_RODADA);
}

void mensagem_fim_partida(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_FIM_PARTIDA);
}

void mensagem_fim_jogo(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_FIM_JOGO);
}

void mensagem_fim_queda(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_FIM_QUEDA);
}

void mensagem_chat(Mensagem *mensagem, char *texto, uint8_t tamanho_texto) {
	mensagem_definir(mensagem, SMT_CHAT, 0, NULL, 0, NULL, (uint8_t *) texto, tamanho_texto);
}

void mensagem_definir_textof(Mensagem *mensagem, const char *format, ...) {
	//mensagem_definir(mensagem, SMT_CHAT, 0, NULL, 0, NULL, NULL, 0);
	
	va_list arg;
	va_start(arg, format);
	vsnprintf((char *) mensagem->dados, BUFF_SIZE, format, arg);
	va_end(arg);
	
	mensagem->tamanho_dados = mensagem->estados * sizeof(EstadoJogador) + mensagem->atualizar_pontuacao * sizeof(EstadoJogo) + strlen((char *) mensagem->dados) + 1;
}

void mensagem_print(const Mensagem *mensagem, const char *titulo) {
	printf("%s Mensagem [%2d (%s)]: '%s'\n", titulo, mensagem->tipo, mensagem_tipo_str[mensagem->tipo], mensagem_obter_texto(mensagem));
}

void mensagem_obter_pontuacao(const Mensagem *mensagem, EstadoJogo *pontuacao) {
	int indice = mensagem->estados * sizeof(EstadoJogador);

	memcpy(pontuacao, &mensagem->dados[indice], sizeof(EstadoJogo));
}

void mensagem_obter_estado_jogadores(const Mensagem *mensagem, EstadoJogador estado_jogadores[NUM_JOGADORES]) {
	int i;
	for (i = 0; i < mensagem->estados; i++) {
		EstadoJogador estado = ((EstadoJogador *)mensagem->dados)[i];
		estado_jogadores[estado.id] = estado;
	}
}

void mensagem_obter_carta(const Mensagem *mensagem, int *indice_carta) {
	memcpy(indice_carta, mensagem->dados, sizeof(Carta));
}

void mensagem_obter_cartas(const Mensagem *mensagem, Carta cartas[NUM_CARTAS_MAO]) {
	memcpy(cartas, mensagem->dados, NUM_CARTAS_MAO * sizeof(Carta));
}

char *mensagem_obter_texto(const Mensagem *mensagem) {
	int indice = mensagem->estados * sizeof(EstadoJogador) + mensagem->atualizar_pontuacao * sizeof(EstadoJogo);
	return (char *) &mensagem->dados[indice];
}
