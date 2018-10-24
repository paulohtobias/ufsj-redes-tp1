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

	return client_socket_fd;
}

void jogador_init(Jogador *jogador, uint8_t id, int sfd) {
	jogador->id = id;
	jogador->socket_fd = sfd;

	gestado_jogadores[jogador->id].id = id;
	gestado_jogadores[jogador->id].qtd_cartas_mao = 0;
	carta_virar(&gestado_jogadores[jogador->id].carta_jogada);

	pthread_create(&jogador->thread_leitura, NULL, t_leitura, jogador);
}

void *t_leitura(void *args) {
	Jogador *jogador = args;

	int retval;
	Mensagem mensagem;

	//Mensagem de boas vindas
	servidor_mensagem_bem_vindo(jogador->id);

	//Envia um sinal para a thread principal para iniciar o jogo
	//caso todos os jogadores já tenham entrado.
	pthread_mutex_lock(&mutex_init);
	num_jogadores++;
	if (num_jogadores == NUM_JOGADORES) {
		pthread_cond_signal(&cond_init);
	}
	pthread_mutex_unlock(&mutex_init);

	// Loop principal
	while (1) {
		#ifdef DEBUG
		printf("cliente %d: thread_leitura-read\n", jogador->id);
		#endif //DEBUG

		//Espera comandos do jogador.
		retval = mensagem_receber(jogador->socket_fd, &mensagem);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		} else if (retval == 0) {
			printf("O cliente %d encerrou a conexão.\n", jogador->id);
			//todo: return NULL;
			exit(0);
		}

		#ifdef DEBUG
		mensagem_print(&mensagem, "chegou do cliente");
		#endif //DEBUG

		if (mensagem.tipo == SMT_SEU_TURNO) {
			pthread_mutex_lock(&mutex_jogo);
			if ((gfase == FJ_TURNO || gfase == FJ_PEDIU_TRUCO) && JOGADOR_ESTA_ATIVO(jogador->id)) {
				mensagem_obter_carta(&mensagem, &gindice_carta);

				//Verifica se o jogador jogou a carta no monte.
				carta_no_monte = 0;
				if (gindice_carta < 0) {
					carta_no_monte = 1;
					gindice_carta *= -1;
				}

				//Verifica se o jogador pediu truco|seis|nove|doze.
				if (gindice_carta == 0) {
					#ifdef DEBUG
					printf("%d pediu truco!\n", jogador->id);
					#endif //DEBUG
					gfase = FJ_PEDIU_TRUCO;
					pthread_cond_signal(&cond_jogo);
				} else {
					gindice_carta--;
					//Verifica se a carta já foi jogada anteriormente.
					if (gjogadores_cartas_jogadas[jogador->id][gindice_carta] == 0) {
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
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_TRUCO) {
			pthread_mutex_lock(&mutex_jogo);
			if (gfase == FJ_PEDIU_TRUCO && JOGADOR_ESTA_ATIVO(jogador->id)) {
				uint8_t resposta;
				mensagem_obter_resposta(&mensagem, &resposta);

				uint8_t indice = (jogador->id > 1);
				gresposta[indice] = resposta;

				#ifdef DEBUG
				printf("RSP (%d) truco: %d\n", jogador->id, resposta);
				#endif //DEBUG

				//Verifica se ambos responderam e se as respostas são iguais.
				if (gresposta[!indice] != RSP_INDEFINIDO && gresposta[0] == gresposta[1]) {
					gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
					pthread_cond_signal(&cond_jogo);
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_MAO_DE_10) {
			pthread_mutex_lock(&mutex_jogo);
			if (gfase == FJ_MAO_DE_10 && JOGADOR_ESTA_ATIVO(jogador->id)) {
				uint8_t resposta;
				mensagem_obter_resposta(&mensagem, &resposta);

				uint8_t indice = (jogador->id > 1);
				gresposta[indice] = resposta;

				#ifdef DEBUG
				printf("RSP (%d) m10: %d\n", jogador->id, resposta);
				#endif //DEBUG

				//Verifica se ambos responderam e se as respostas são iguais.
				if (gresposta[!indice] <= RSP_SIM && gresposta[0] == gresposta[1]) {
					pthread_cond_signal(&cond_jogo);
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_FIM_QUEDA) {
			pthread_mutex_lock(&mutex_jogo);
			if (gfase == FJ_FIM_QUEDA && JOGADOR_ESTA_ATIVO(jogador->id)) {
				uint8_t resposta;
				mensagem_obter_resposta(&mensagem, &resposta);

				uint8_t indice = JOGADOR_TIME(jogador->id);
				gresposta[indice] = resposta;

				#ifdef DEBUG
				printf("(%d) RSP fim jogo : %d\n", jogador->id, resposta);
				#endif //DEBUG

				//Verifica se ambos responderam e se as respostas são iguais.
				if (gresposta[!indice] <= RSP_SIM && gresposta[0] == gresposta[1]) {
					pthread_cond_signal(&cond_jogo);
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_CHAT) {
			servidor_mensagem_chat(mensagem_obter_texto(&mensagem, NULL), mensagem.tamanho_dados);
		}
	}
}

void enviar_mensagem(const Mensagem *mensagem, uint8_t new_msg) {
	int i, retval;
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Só envia para quem está autorizado a receber.
		if (((1 << i) & new_msg) && jogadores[i].id != -1) {
			#ifdef DEBUG
			//printf("enviando msg %d (%s) para %d\n", mensagem->tipo, mensagem_tipo_str[mensagem->tipo], jogadores[i].id);
			#endif //DEBUG
			
			retval = mensagem_enviar(mensagem, jogadores[i].socket_fd);
			if (retval == -1) {
				handle_error(1, "enviar_mensagem-write");
			}
			
			#ifdef DEBUG
			//printf("escreveu %d bytes na msg %d praa %d\n", retval, mensagem->tipo, jogadores[i].id);
			#endif //DEBUG
		}
	}
}

int avisar_truco(int8_t jogador_id) {
	pthread_mutex_lock(&mutex_jogo);
	
	//Avisa para o time que o jogador pediu truco.
	char texto[BUFF_SIZE];
	sprintf(texto, "Seu time pediu %s. Aguardando resposta do time adversário.", valor_partida_str[gestado.valor_partida + 1]);
	servidor_mensagem_processando(MSG_JOGADOR(jogador_id), texto);
	
	//Avisa ao outro time que o jogador pediu truco e espera pela reposta.
	gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
	servidor_mensagem_truco(jogador_id);
	pthread_mutex_unlock(&mutex_jogo);


	pthread_mutex_lock(&mutex_jogo);
	//Aguarda a jogada.
	while (gresposta[0] == RSP_INDEFINIDO || gresposta[0] != gresposta[1]) {
		#ifdef DEBUG
		printf("Aguardando o resposta do truco...\n");
		#endif //DEBU
		
		pthread_cond_wait(&cond_jogo, &mutex_jogo);
	}
	
	//Repostas foram dadas.
	if (gresposta[0] == RSP_NAO) {
		#ifdef DEBUG
		printf("o time %d correu do truco|seis|nove|doze\n", JOGADOR_TIME_ADV(jogador_id));
		#endif //DEBUG
		
		gvencedor_partida = JOGADOR_TIME(jogador_id);
		pthread_mutex_unlock(&mutex_jogo);
		return RSP_NAO;
	} else if (gresposta[0] == RSP_SIM) {
		gestado.valor_partida++;

		gestado.time_truco = JOGADOR_TIME(jogador_id);

		//Atualização geral.
		servidor_mensagem_atualizar_estado("Truco|Seis|Nove|Doze Aceito.");

		#ifdef DEBUG
		printf("o time %d aceitou o truco|seis|nove|doze\n", JOGADOR_TIME_ADV(jogador_id));
		#endif //DEBUG

		pthread_mutex_unlock(&mutex_jogo);
		return RSP_SIM;
	} else { //Se cair aqui, então a reposta foi pra aumentar a aposta.

		//Calcula o id que será mostrado aos jogadores do outro time como quem aumentou.
		//Varia entre o jogador atual e quem está a sua direita (sentido anti-horário).
		int8_t jogador_id_adv;
		if (jogador_id == gestado.jogador_atual) {
			jogador_id_adv = (gestado.jogador_atual + 1) % NUM_JOGADORES;
		} else {
			jogador_id_adv = gestado.jogador_atual;
		}

		#ifdef DEBUG
		printf("AUMENTOU: (%d, %d)\n", jogador_id, gestado.jogador_atual);
		printf("o jogador %d aumentou!! truco|seis|nove|doze\n", jogador_id_adv);
		#endif //DEBUG
		
		//Aumenta o valor da partida e avisa a todos os jogadores.
		gestado.valor_partida++;
		servidor_mensagem_atualizar_estado(NULL);

		pthread_mutex_unlock(&mutex_jogo);
		return avisar_truco(jogador_id_adv);
	}
}


void servidor_mensagem_bem_vindo(int8_t id) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_bem_vindo(&gmensagem, id);
	new_msg = MSG_JOGADOR(id);
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_processando(uint8_t remetentes, const char *texto) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_processando(&gmensagem, texto);
	new_msg = remetentes;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_atualizar_estado(const char *texto) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
	if (texto != NULL) {
		mensagem_definir_textof(&gmensagem, texto);
	}
	new_msg = MSG_TODOS;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_enviar_cartas(int8_t id) {
	pthread_mutex_lock(&mutex_broadcast);
	
	/*todo:
	Carta cartas[NUM_CARTAS_MAO];
	memcpy(cartas, gjogadores_cartas[id], sizeof(Carta));
	if (MAO_DE_FERRO) {
		for (int i = 0; i < NUM_CARTAS_MAO; i++) {
			cartas_virar(&cartas[i]);
		}
	}
	*/
	
	mensagem_definir_cartas(&gmensagem, gjogadores_cartas[id]);
	new_msg = MSG_JOGADOR(jogadores[id].id);
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_seu_turno() {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_seu_turno(&gmensagem);
	new_msg = MSG_JOGADOR(gestado.jogador_atual);
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_aguardar_turno() {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_aguardar_turno(&gmensagem, gestado.jogador_atual);
	new_msg = MSG_TODOS - MSG_JOGADOR(gestado.jogador_atual);
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_jogada_aceita() {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_jogada_aceita(&gmensagem, &gestado, gindice_carta);
	new_msg = MSG_JOGADOR(gestado.jogador_atual);
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_truco(int8_t id) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_truco(&gmensagem, id);
	uint8_t time_adversario = MSG_TIME_ADV(id);
	gjogadores_ativos = time_adversario;
	enviar_mensagem(&gmensagem, time_adversario);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_empate() {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_empate(&gmensagem);
	new_msg = MSG_TODOS;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_mao_de_10(uint8_t time) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_mao_de_10(&gmensagem);
	new_msg = time ? MSG_TIME2 : MSG_TIME1;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
	pthread_mutex_unlock(&mutex_jogo);
}

void servidor_mensagem_fim_queda() {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_fim_queda(&gmensagem, gvencedor_queda);
	new_msg = MSG_TODOS;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}

void servidor_mensagem_chat(const char *texto, uint8_t tamanho_dados) {
	pthread_mutex_lock(&mutex_broadcast);
	mensagem_chat(&gmensagem, texto, tamanho_dados);
	new_msg = MSG_TODOS;
	enviar_mensagem(&gmensagem, new_msg);
	pthread_mutex_unlock(&mutex_broadcast);
}
