#include "protocolo.h"

char mensagem_tipo_str[__SMT_QTD][BUFF_SIZE] = {
	"Houve um erro no sistema.",
	"Bem vindo, %s! Aguardando todos os jogadores...",
	"Processando.",
	"Estou te enviando suas cartas.",
	"Seu turno.",
	"Jogada aceita.",
	"%s pediu truco|seis|nove|doze. (0:não; 1: sim; 2: aumento)",
	"Houve empate. Me mostre sua maior carta.",
	"Fim da rodada.",
	"Fim da partida.",
	"Fim do jogo.",
	"Fim da queda.",
	"Nova mensagem no chat."
};


int mensagem_receber(int sfd, Mensagem *mensagem) {
	return read(sfd, mensagem, sizeof(Mensagem));
}

int mensagem_enviar(const Mensagem *mensagem, int sfd) {
	return write(sfd, mensagem, mensagem_obter_tamanho(mensagem));
}


size_t mensagem_obter_tamanho(const Mensagem *mensagem) {
	//return ((void *) mensagem->dados - (void *) mensagem) + mensagem->tamanho_dados;
	return sizeof(Mensagem);
}

void mensagem_definir(Mensagem *mensagem, MENSAGEM_TIPO tipo, uint8_t atualizar_estado_jogo, uint8_t atualizar_estado_jogadores, uint8_t tamanho_dados, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES], const void *dados) {
	//Metadados.
	mensagem->tipo = tipo;
	mensagem->atualizar_estado_jogo = estado_jogo != NULL;
	mensagem->atualizar_estado_jogadores = estado_jogadores != NULL;
	mensagem->tamanho_dados = tamanho_dados;

	
	//Dados.
	memset(&mensagem->estado_jogo, 0, sizeof(EstadoJogo));
	memset(mensagem->estado_jogadores, 0, sizeof(EstadoJogador) * NUM_JOGADORES);
	memset(mensagem->dados, 0, BUFF_SIZE);
	
	if (atualizar_estado_jogo) {
		memcpy(&mensagem->estado_jogo, estado_jogo, sizeof(EstadoJogo));
	}
	if (atualizar_estado_jogadores) {
		memcpy(mensagem->estado_jogadores, estado_jogadores, sizeof(EstadoJogador) * NUM_JOGADORES);
	}
	memcpy(mensagem->dados, dados, mensagem->tamanho_dados);
}

void mensagem_simples(Mensagem *mensagem, MENSAGEM_TIPO tipo) {
	mensagem_definir(mensagem, tipo, 0, 0, 0, NULL, NULL, NULL);
}

void mensagem_atualizar_estado(Mensagem *mensagem, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES]) {
	mensagem_definir(mensagem, SMT_PROCESSANDO, 1, 1, 0, estado_jogo, estado_jogadores, NULL);
}

void mensagem_somente_dados(Mensagem *mensagem, MENSAGEM_TIPO tipo, uint8_t tamanho_dados, const void *dados) {
	mensagem_definir(mensagem, tipo, 0, 0, tamanho_dados, NULL, NULL, dados);
}


void mensagem_obter_id(const Mensagem *mensagem, int8_t *id) {
	memcpy(id, mensagem->dados, sizeof *id);
}

void mensagem_bem_vindo(Mensagem *mensagem, int8_t id) {
	mensagem_somente_dados(mensagem, SMT_BEM_VINDO, sizeof id, &id);
}

void mensagem_obter_estado_jogo(const Mensagem *mensagem, EstadoJogo *estado_jogo) {
	memcpy(estado_jogo, &mensagem->estado_jogo, sizeof(EstadoJogo));
}

void mensagem_obter_estado_jogadores(const Mensagem *mensagem, EstadoJogador estado_jogadores[NUM_JOGADORES]) {
	memcpy(estado_jogadores, mensagem->estado_jogadores, sizeof(EstadoJogador) * NUM_JOGADORES);
}

void mensagem_definir_cartas(Mensagem *mensagem, const Carta cartas[NUM_CARTAS_MAO]) {
	mensagem_somente_dados(mensagem, SMT_ENVIANDO_CARTAS, NUM_CARTAS_MAO * sizeof(Carta), cartas);
}

void mensagem_obter_cartas(const Mensagem *mensagem, Carta cartas[NUM_CARTAS_MAO]) {
	memcpy(cartas, mensagem->dados, NUM_CARTAS_MAO * sizeof(Carta));
}

void mensagem_obter_carta(const Mensagem *mensagem, int8_t *indice_carta) {
	memcpy(indice_carta, mensagem->dados, sizeof *indice_carta);
}

void mensagem_definir_carta(Mensagem *mensagem, int8_t indice_carta) {
	mensagem_somente_dados(mensagem, mensagem->tipo, sizeof indice_carta, &indice_carta);
}

void mensagem_obter_truco_id(const Mensagem *mensagem, int8_t *id) {
	memcpy(id, mensagem->dados, sizeof *id);
}

void mensagem_truco(Mensagem *mensagem, int8_t id) {
	mensagem_somente_dados(mensagem, SMT_TRUCO, sizeof id, &id);
}

void mensagem_obter_resposta(const Mensagem *mensagem, uint8_t *resposta) {
	memcpy(resposta, mensagem->dados, sizeof *resposta);
}

void mensagem_definir_resposta(Mensagem *mensagem, RESPOSTAS resposta) {
	mensagem_somente_dados(mensagem, mensagem->tipo, sizeof resposta, &resposta);
}

char *mensagem_obter_texto(const Mensagem *mensagem, char *texto) {
	if (texto != NULL) {
		strcpy(texto, (char *) mensagem->dados);
		return texto;
	}
	return (char *) mensagem->dados;
}

void mensagem_definir_textof(Mensagem *mensagem, const char *format, ...) {
	va_list arg;
	va_start(arg, format);
	vsnprintf((char *) mensagem->dados, BUFF_SIZE, format, arg);
	va_end(arg);
	
	mensagem->tamanho_dados = strlen((char *) mensagem->dados) + 1;
}


void mensagem_processando(Mensagem *mensagem, const char *texto) {
	mensagem_somente_dados(mensagem, SMT_PROCESSANDO, strlen(texto) + 1, texto);
}

void mensagem_seu_turno(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_SEU_TURNO);
}

void mensagem_jogada_aceita(Mensagem *mensagem, const EstadoJogo *estado_jogo, int8_t indice_carta) {
	mensagem_definir(mensagem, SMT_JOGADA_ACEITA, 1, 0, sizeof indice_carta, estado_jogo, NULL, &indice_carta);
}

void mensagem_empate(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_EMPATE);
}

void mensagem_fim_rodada(Mensagem *mensagem, uint8_t time_vencedor) {
	mensagem_simples(mensagem, SMT_FIM_RODADA);
}

void mensagem_fim_partida(Mensagem *mensagem, uint8_t time_vencedor) {
	mensagem_simples(mensagem, SMT_FIM_PARTIDA);
}

void mensagem_fim_jogo(Mensagem *mensagem, uint8_t time_vencedor) {
	mensagem_simples(mensagem, SMT_FIM_JOGO);
}

void mensagem_fim_queda(Mensagem *mensagem, uint8_t time_vencedor) {
	mensagem_simples(mensagem, SMT_FIM_QUEDA);
}

void mensagem_chat(Mensagem *mensagem, const char *texto, uint8_t tamanho_texto) {
	mensagem_somente_dados(mensagem, SMT_CHAT, tamanho_texto, texto);
}


void mensagem_print(const Mensagem *mensagem, const char *titulo) {
	//printf("%s Mensagem [%2d (%s)]: '%s'\n", titulo, mensagem->tipo, mensagem_tipo_str[mensagem->tipo], mensagem_obter_texto(mensagem, NULL));
	printf("%s Mensagem [%2d]\n", titulo, mensagem->tipo);
}