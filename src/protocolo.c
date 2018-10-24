#include "protocolo.h"

char mensagem_tipo_str[__SMT_QTD][BUFF_SIZE] = {
	"Houve um erro no sistema.",
	"Bem vindo, %s! Aguardando todos os jogadores...",
	"Processando.",
	"Estou te enviando suas cartas.",
	"Seu turno.",
	"Aguardando a jogada do Jogador %d",
	"Jogada aceita.",
	"%s pediu %s. (0: não; 1: sim; 2: aumento)",
	"Houve empate. Me mostre sua maior carta.",
	"Mão de 10. Você aceita? (0: não; 1: sim)",
	"O time %d foi o grande vencedor da queda!!! Deseja jogar novamente? (0: não; 1: sim)",
	"Nova mensagem no chat."
};


int mensagem_receber(int sfd, Mensagem *mensagem) {
	memset(mensagem, 0, sizeof(Mensagem));
	return read(sfd, mensagem, sizeof(Mensagem));
}

int mensagem_enviar(const Mensagem *mensagem, int sfd) {
	return write(sfd, mensagem, mensagem_obter_tamanho(mensagem));
}


size_t mensagem_obter_tamanho(const Mensagem *mensagem) {
	return sizeof(Mensagem);
}

void mensagem_definir(Mensagem *mensagem, MENSAGEM_TIPO tipo, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES], const void *dados, uint8_t tamanho_dados) {
	//Metadados.
	mensagem->tipo = tipo;
	mensagem->atualizar_estado_jogo = estado_jogo != NULL;
	mensagem->atualizar_estado_jogadores = estado_jogadores != NULL;
	mensagem->tamanho_dados = tamanho_dados;

	
	//Dados.
	memset(&mensagem->estado_jogo, 0, sizeof(EstadoJogo));
	memset(mensagem->estado_jogadores, 0, sizeof(EstadoJogador) * NUM_JOGADORES);
	memset(mensagem->dados, 0, BUFF_SIZE);
	
	if (mensagem->atualizar_estado_jogo) {
		memcpy(&mensagem->estado_jogo, estado_jogo, sizeof(EstadoJogo));
	}
	if (mensagem->atualizar_estado_jogadores) {
		memcpy(mensagem->estado_jogadores, estado_jogadores, sizeof(EstadoJogador) * NUM_JOGADORES);
	}
	memcpy(mensagem->dados, dados, mensagem->tamanho_dados);
}

void mensagem_simples(Mensagem *mensagem, MENSAGEM_TIPO tipo) {
	mensagem_definir(mensagem, tipo, NULL, NULL, NULL, 0);
}

void mensagem_atualizar_estado(Mensagem *mensagem, const EstadoJogo *estado_jogo, const EstadoJogador estado_jogadores[NUM_JOGADORES]) {
	mensagem_definir(mensagem, SMT_PROCESSANDO, estado_jogo, estado_jogadores, NULL, 0);
}

void mensagem_somente_dados(Mensagem *mensagem, MENSAGEM_TIPO tipo, const void *dados, uint8_t tamanho_dados) {
	mensagem_definir(mensagem, tipo, NULL, NULL, dados, tamanho_dados);
}


void mensagem_obter_id(const Mensagem *mensagem, int8_t *id) {
	memcpy(id, mensagem->dados, sizeof *id);
}

void mensagem_bem_vindo(Mensagem *mensagem, int8_t id) {
	mensagem_somente_dados(mensagem, SMT_BEM_VINDO, &id, sizeof id);
}

void mensagem_obter_estado_jogo(const Mensagem *mensagem, EstadoJogo *estado_jogo) {
	memcpy(estado_jogo, &mensagem->estado_jogo, sizeof(EstadoJogo));
}

void mensagem_obter_estado_jogadores(const Mensagem *mensagem, EstadoJogador estado_jogadores[NUM_JOGADORES]) {
	memcpy(estado_jogadores, mensagem->estado_jogadores, sizeof(EstadoJogador) * NUM_JOGADORES);
}

void mensagem_definir_cartas(Mensagem *mensagem, const Carta cartas[NUM_CARTAS_MAO]) {
	mensagem_somente_dados(mensagem, SMT_ENVIANDO_CARTAS, cartas, NUM_CARTAS_MAO * sizeof(Carta));
}

void mensagem_obter_cartas(const Mensagem *mensagem, Carta cartas[NUM_CARTAS_MAO]) {
	memcpy(cartas, mensagem->dados, NUM_CARTAS_MAO * sizeof(Carta));
}

void mensagem_obter_carta(const Mensagem *mensagem, int8_t *indice_carta) {
	memcpy(indice_carta, mensagem->dados, sizeof *indice_carta);
}

void mensagem_definir_carta(Mensagem *mensagem, int8_t indice_carta) {
	mensagem_somente_dados(mensagem, mensagem->tipo, &indice_carta, sizeof indice_carta);
}

void mensagem_aguardar_turno(Mensagem *mensagem, int8_t id) {
	mensagem_somente_dados(mensagem, SMT_AGUARDANDO_TURNO, &id, sizeof id);
}

void mensagem_truco(Mensagem *mensagem, int8_t id) {
	mensagem_somente_dados(mensagem, SMT_TRUCO, &id, sizeof id);
}

void mensagem_obter_resposta(const Mensagem *mensagem, uint8_t *resposta) {
	memcpy(resposta, mensagem->dados, sizeof *resposta);
}

void mensagem_definir_resposta(Mensagem *mensagem, RESPOSTA resposta) {
	mensagem_somente_dados(mensagem, mensagem->tipo, &resposta, sizeof resposta);
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
	uint8_t tamanho_texto = 0;
	if (texto != NULL) {
		tamanho_texto = strlen(texto) + 1;
	}
	mensagem_somente_dados(mensagem, SMT_PROCESSANDO, texto, tamanho_texto);
}

void mensagem_seu_turno(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_SEU_TURNO);
}

void mensagem_jogada_aceita(Mensagem *mensagem, const EstadoJogo *estado_jogo, int8_t indice_carta) {
	mensagem_definir(mensagem, SMT_JOGADA_ACEITA, estado_jogo, NULL, &indice_carta, sizeof indice_carta);
}

void mensagem_empate(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_EMPATE);
}

void mensagem_mao_de_10(Mensagem *mensagem) {
	mensagem_simples(mensagem, SMT_MAO_DE_10);
}

void mensagem_fim_queda(Mensagem *mensagem, const EstadoJogo *estado, uint8_t time_vencedor) {
	mensagem_definir(mensagem, SMT_FIM_QUEDA, estado, NULL, &time_vencedor, sizeof time_vencedor);
}

void mensagem_chat(Mensagem *mensagem, const char *texto, uint8_t tamanho_texto) {
	mensagem_somente_dados(mensagem, SMT_CHAT, texto, tamanho_texto);
}


void mensagem_print(const Mensagem *mensagem, const char *titulo) {
	//printf("%s Mensagem [%2d (%s)]: '%s'\n", titulo, mensagem->tipo, mensagem_tipo_str[mensagem->tipo], mensagem_obter_texto(mensagem, NULL));
	printf("%s Mensagem [%2d]\n", titulo, mensagem->tipo);
}