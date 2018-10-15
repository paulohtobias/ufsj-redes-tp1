#include "servidor.h"

pthread_mutex_t mutex_jogo = PTHREAD_MUTEX_INITIALIZER;
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

	jogador->estado.id = id;
	jogador->estado.qtd_cartas_mao = 0;
	carta_virar(&jogador->estado.carta_jogada);

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
		"<span font_weight='bold' color='%s'>Jogador %d: </span>",
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
			if (gmao == jogador->id) {
				int indice_carta;
				mensagem_obter_carta(&mensagem, &indice_carta);

				if (indice_carta < NUM_CARTAS_MAO && gjogadores_cartas_jogadas[jogador->id][indice_carta] == 0) {
					//Marca a carta como jogada.
					gjogadores_cartas_jogadas[jogador->id][indice_carta] = 1;
					
					//Atualiza o estado do jogador.
					jogador->estado.carta_jogada = gjogadores_cartas[jogador->id][indice_carta];
					jogador->estado.qtd_cartas_mao--;

					//Atualiza a carta mais forte
					if (jogador->estado.carta_jogada.poder > gcarta_mais_forte.poder) {
						gcarta_mais_forte = jogador->estado.carta_jogada;
						gjogador_carta_mais_forte = jogador->id;
						gempate_parcial = 0;
					} else if (jogador->estado.carta_jogada.poder == gcarta_mais_forte.poder) {
						gcarta_mais_forte = jogador->estado.carta_jogada;
						gjogador_carta_mais_forte = jogador->id;
						gempate_parcial = 1;
					}

					pthread_mutex_lock(&mutex_broadcast);

					gturno++;
					//Verifica se chegou no fim da rodada.
					if (gturno == NUM_JOGADORES) {
						gvencedor_partida = terminar_rodada();

						if (gvencedor_partida == -1) {
							mensagem_fim_rodada(&gmensagem);
						} else {
							gvencedor_jogo = terminar_partida();
							
							if (gvencedor_jogo == -1) {
								mensagem_fim_partida(&gmensagem);
							} else {
								gvencedor_queda = terminar_jogo();

								if (gvencedor_queda == -1) {
									mensagem_fim_jogo(&gmensagem);
								} else {
									mensagem_fim_queda(&mensagem);
								}
							}
						}
					} else {
						mensagem_atualizar_estado(&gmensagem, 1, &jogador->estado, 0, NULL);
					}

					new_msg = MSG_TODOS;
					pthread_cond_signal(&cond_new_msg);
					pthread_mutex_unlock(&mutex_broadcast);
				} else if (indice_carta == NUM_CARTAS_MAO) {
					// PEDIU TRUCO.
					#ifdef DEBUG
					printf("Jogador %d pediu truco\n", jogador->id);
					#endif //DEBUG
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		} else if (mensagem.tipo == SMT_CHAT) {
			pthread_mutex_lock(&mutex_broadcast);
			
			mensagem_chat(&gmensagem, NULL, 0);
			mensagem_definir_textof(&gmensagem, "%s%s\n", jogador_nome, mensagem.dados);
			
			new_msg = MSG_TODOS;
			pthread_cond_signal(&cond_new_msg);
			pthread_mutex_unlock(&mutex_broadcast);
		}

		if (jogador->thread.new_msg) {
		}
	}
}

void *t_escrita(void *args) {
	int i;

	while (1) {
		//Espera uma nova mensagem chegar.
		pthread_mutex_lock(&mutex_new_msg);
		while (new_msg == MSG_NINGUEM) {
			pthread_cond_wait(&cond_new_msg, &mutex_new_msg);
		}
		pthread_mutex_unlock(&mutex_new_msg);

		//Envia a mensagem aos clientes.
		pthread_mutex_lock(&mutex_broadcast);
		for (i = 0; i < NUM_JOGADORES; i++) {
			if (((1 << i) & new_msg) && jogadres[i].id != -1) {
				#ifdef DEBUG
				printf("[Servidor] enviando msg (%d) para %d\n", gmensagem.tipo, i);
				#endif //DEBUG
				write(jogadres[i].socket_fd, &gmensagem, mensagem_obter_tamanho(&gmensagem));
			}
		}
		new_msg = MSG_NINGUEM;
		pthread_mutex_unlock(&mutex_broadcast);
	}
}
