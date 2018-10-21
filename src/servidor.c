#include "servidor.h"

pthread_mutex_t mutex_jogo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_jogo = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_init = PTHREAD_COND_INITIALIZER;
int num_jogadores = 0;

pthread_mutex_t mutex_broadcast = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_new_msg = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_new_msg = PTHREAD_COND_INITIALIZER;
uint8_t new_msg = MSG_NINGUEM;

int criar_socket_servidor() {
	return criar_socket(INADDR_ANY, PORTA, CONEXAO_MODO_SERVIDOR);
}

int s_accept(int ssfd) {
	int client_socket_fd;
	socklen_t client_len;
	struct sockaddr_in client_addr;

	client_len = sizeof(client_addr);

	client_socket_fd = accept(ssfd, (struct sockaddr *) &client_addr, &client_len);

	if (client_socket_fd == -1) {
		handle_error(client_socket_fd, "accept");
	}

	num_jogadores++;

	printf("[Server]: client %d connected.\n", client_socket_fd);

	return client_socket_fd;
}

void jogador_init(Jogador *jogador, uint8_t id, int sfd) {
	jogador->id = id;
	jogador->socket_fd = sfd;

	gestado_jogadores[jogador->id].id = id;
	gestado_jogadores[jogador->id].qtd_cartas_mao = 0;
	carta_virar(&gestado_jogadores[jogador->id].carta_jogada);

	jogador->thread.new_msg = 0;
	pthread_mutex_init(&jogador->thread.new_msg_mutex, NULL);
	pthread_cond_init(&jogador->thread.new_msg_cond, NULL);

	pthread_create(&jogador->thread.leitura, NULL, t_leitura, jogador);
	//pthread_create(&jogador->thread.escrita, NULL, t_escrita, jogador);
}

void *t_leitura(void *args) {
	Jogador *jogador = args;

	int retval;
	char jogador_nome[128];
	Mensagem mensagem;

	//Setando o nome do jogador
	sprintf(
		jogador_nome,
		"<span font_weight='bold' color='%s'>Jogador %d</span>",
		cores_times[jogador->id], jogador->id
	);

	//Mensagem de boas vindas
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_bem_vindo(&gmensagem);
	mensagem_definir_textof(&gmensagem, "Bem vindo, %s! Aguardando todos os jogadores...", jogador_nome);
	new_msg = MSG_JOGADOR(jogador->id);
	pthread_cond_signal(&cond_new_msg);
	pthread_mutex_unlock(&mutex_broadcast);

	// Loop principal
	while (1) {
		#ifdef DEBUG
		printf("client %d: thread_leitura-read\n", jogador->id);
		#endif //DEBUG

		//Espera comandos do jogador.
		memset(&mensagem, 0, sizeof mensagem);
		retval = read(jogador->socket_fd, &mensagem, sizeof mensagem);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		} else if (retval == 0) {
			printf("O cliente %d encerrou a conexão.\n", jogador->id);
			//todo: return NULL;
			exit(0);
		}

		#ifdef DEBUG
		mensagem_print(&mensagem, "");
		#endif //DEBUG

		if (mensagem.tipo == SMT_SEU_TURNO) {
			pthread_mutex_lock(&mutex_jogo);
			if ((gfase == FJ_TURNO || gfase == FJ_PEDIU_TRUCO) && JOGADOR_ESTA_ATIVO(jogador->id)) {
				mensagem_obter_carta(&mensagem, &gindice_carta);

				//Verifica se o jogador jogou a carta no monte.
				carta_no_monte = 0;
				if (gindice_carta < 0) {
					carta_no_monte = 1;
					gindice_carta = -(1 + gindice_carta);
				}

				if (gindice_carta == NUM_JOGADORES) {
					gestado.time_truco = JOGADOR_TIME(jogador->id);
					gfase = FJ_PEDIU_TRUCO;
					pthread_cond_signal(&cond_jogo);
				}
				//Verifica se a carta já foi jogada anteriormente.
				else if (gjogadores_cartas_jogadas[jogador->id][gindice_carta] == 0) {
					//Marca a carta como jogada.
					gjogadores_cartas_jogadas[jogador->id][gindice_carta] = 1;

					//Vira a carta do jogador, se necessário.
					if (carta_no_monte) {
						carta_virar(&gjogadores_cartas[jogador->id][gindice_carta]);
					}

					//Atualiza o estado do jogador.
					gestado_jogadores[jogador->id].carta_jogada = gjogadores_cartas[jogador->id][gindice_carta];
					gestado_jogadores[jogador->id].qtd_cartas_mao--;

					gfase = FJ_FIM_TURNO;
					pthread_cond_signal(&cond_jogo);
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_TRUCO) {
			if (FJ_PEDIU_TRUCO && JOGADOR_ESTA_ATIVO(jogador->id)) {
				uint8_t resposta;
				mensagem_obter_resposta(&mensagem, &resposta);

				uint8_t indice = (jogador->id > 1);
				pthread_mutex_lock(&mutex_jogo);
				gresposta[indice] = resposta;

				//Verifica se ambos responderam e se as respostas são iguais.
				if (gresposta[!indice] != RSP_INDEFINIDO && gresposta[0] == gresposta[1]) {
					gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
					//gfase = FJ_TURNO;
					pthread_cond_signal(&cond_jogo);
				}
				pthread_mutex_unlock(&mutex_jogo);
			}
		} else if (mensagem.tipo == SMT_CHAT) {
			pthread_mutex_lock(&mutex_broadcast);
			
			mensagem_chat(&gmensagem, NULL, 0);
			mensagem_definir_textof(&gmensagem, "%s: %s\n", jogador_nome, mensagem.dados);
			
			new_msg = MSG_TODOS;
			pthread_cond_signal(&cond_new_msg);
			pthread_mutex_unlock(&mutex_broadcast);
		}

		if (jogador->thread.new_msg) {
		}
	}
}

void *t_escrita(void *args) {
	while (1) {
		//Espera uma nova mensagem chegar.
		pthread_mutex_lock(&mutex_new_msg);
		while (new_msg == MSG_NINGUEM) {
			pthread_cond_wait(&cond_new_msg, &mutex_new_msg);
		}
		pthread_mutex_unlock(&mutex_new_msg);

		//Envia a mensagem aos clientes.
		pthread_mutex_lock(&mutex_broadcast);
		enviar_mensagem(&gmensagem, new_msg);
		new_msg = MSG_NINGUEM;
		pthread_mutex_unlock(&mutex_broadcast);
	}
}

void enviar_mensagem(const Mensagem *mensagem, uint8_t new_msg) {
	int i, retval;
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Só envia para quem está autorizado a receber.
		if (((1 << i) & new_msg) && jogadores[i].id != -1) {
			#ifdef DEBUG
			printf("[Servidor] enviando msg %d (%s) para %d\n", mensagem->tipo, mensagem_tipo_str[mensagem->tipo], jogadores[i].id);
			#endif //DEBUG
			
			retval = write(jogadores[i].socket_fd, mensagem, mensagem_obter_tamanho(mensagem));
			
			#ifdef DEBUG
			printf("[Servidor] escreveu %d bytes na msg %d praa %d\n", retval, mensagem->tipo, jogadores[i].id);
			#endif //DEBUG
		}
	}
}
