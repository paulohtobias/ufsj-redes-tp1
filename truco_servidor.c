#include "servidor.h"
#include <time.h>

int main(int argc, char *argv[]) {
	int i, j;

	srand((unsigned) time(NULL));

	int chat_socket_fd = criar_socket_servidor();

	for (i = 0; i < NUM_JOGADORES; i++) {
		jogadores[i].id = -1;
	}
	
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(chat_socket_fd);

		jogador_init(jogadores + i, i, jsfd);
	}

	//sleep(3);
	printf("\n\n");

	pthread_mutex_lock(&mutex_jogo);
	gvencedor_jogo = -1;
	gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
	gmao = rand() % NUM_JOGADORES;
	pthread_mutex_unlock(&mutex_jogo);
	
	//Iniciando o jogo.
	pthread_mutex_lock(&mutex_init);
	while (num_jogadores < NUM_JOGADORES) {
		pthread_cond_wait(&cond_init, &mutex_init);
	}
	pthread_mutex_unlock(&mutex_init);
	
	//Loop infinito até que os jogadores fechem o jogo.
	while (1) {
		
		//While da queda. Cada iteração é um jogo.
		while (1) {

			//While do jogo: cada iteração é uma partida.
			while (1) {
				//Embaralha as cartas.
				embaralhar(gbaralho, NUM_CARTAS);
				
				//Muda a fase do jogo.
				pthread_mutex_lock(&mutex_jogo);
				gfase = FJ_ENVIANDO_CARTAS;
				
				//Distribui as cartas a todos os jogadores.
				for (i = 0; i < NUM_JOGADORES; i++) {
					for (j = 0; j < NUM_CARTAS_MAO; j++) {
						gjogadores_cartas[i][j] = gbaralho[i * NUM_JOGADORES + j];
						gjogadores_cartas_jogadas[i][j] = 0;
					}
					carta_virar(&gestado_jogadores[i].carta_jogada);
					gestado_jogadores[i].qtd_cartas_mao = 3;
					
					//Avisa o jogador.
					pthread_mutex_lock(&mutex_broadcast);
					mensagem_definir_cartas(&gmensagem, gjogadores_cartas[i]);
					new_msg = MSG_JOGADOR(jogadores[i].id);
					#ifdef DEBUG
					printf("Enviando cartas para o jogador %d\n", jogadores[i].id);
					#endif //DEBUG
					enviar_mensagem(&gmensagem, new_msg);
					pthread_mutex_unlock(&mutex_broadcast);
				}
				pthread_mutex_unlock(&mutex_jogo);

				//Atualiza o estado de todos os jogadores.
				pthread_mutex_lock(&mutex_broadcast);
				mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
				new_msg = MSG_TODOS;
				enviar_mensagem(&gmensagem, new_msg);
				pthread_mutex_unlock(&mutex_broadcast);

				//While da partida: cada iteração é uma rodada (JOGO)
				//iniciar_rodada();
				while (1) {

					pthread_mutex_lock(&mutex_jogo);
					gturno = 0;
					gestado.jogador_atual = gmao;
					
					#ifdef DEBUG
					printf("[JOGO] Inicio do turno.\n$$$ Jogador atual: %d\n", gestado.jogador_atual);
					#endif //DEBUG
					
					pthread_mutex_unlock(&mutex_jogo);
					//While da rodada: cada iteração é um turno.
					while (1) {
						pthread_mutex_lock(&mutex_jogo);

						//Marca a fase do jogo como TURNO.
						gfase = FJ_TURNO;

						//Seta o jogador atual como o único jogador ativo.
						gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
						
						//Avisa para o jogador que é sua vez de jogar.
						pthread_mutex_lock(&mutex_broadcast);
						mensagem_seu_turno(&gmensagem);
						new_msg = MSG_JOGADOR(gestado.jogador_atual);
						enviar_mensagem(&gmensagem, new_msg);
						pthread_mutex_unlock(&mutex_broadcast);
						pthread_mutex_unlock(&mutex_jogo);


						pthread_mutex_lock(&mutex_jogo);
						//Aguarda a jogada.
						while (gfase != FJ_FIM_TURNO && gfase != FJ_PEDIU_TRUCO) {
							pthread_cond_wait(&cond_jogo, &mutex_jogo);
						}
						//Jogada foi feita.

						if (gfase == FJ_PEDIU_TRUCO) {
							if (gestado.mao_de_10) {
								#ifdef DEBUG
								printf("%d pediu truco na mao de 10\n", gestado.jogador_atual);
								#endif //DEBUG
								
								gvencedor_partida = !gestado.jogador_atual;
								pthread_mutex_unlock(&mutex_jogo);
								break;
							}
							mensagem_truco(&gmensagem);
							gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
							uint8_t time_adversario = (JOGADOR_TIME(gestado.jogador_atual) == 0) ? MSG_TIME1 : MSG_TIME2;
							gjogadores_ativos = time_adversario;
							enviar_mensagem(&gmensagem, time_adversario);
							pthread_mutex_unlock(&mutex_jogo);

							pthread_mutex_lock(&mutex_jogo);
							//Aguarda a jogada.
							while (gresposta[0] == RSP_INDEFINIDO || gresposta[0] != gresposta[1]) {
								pthread_cond_wait(&cond_jogo, &mutex_jogo);
							}
							//Repostas foram dadas.
							if (gresposta[0] == RSP_NAO) {
								#ifdef DEBUG
								printf("o time %d correu do truco|seis|nove|doze\n", !gestado.jogador_atual);
								#endif //DEBUG
								
								gvencedor_partida = gestado.jogador_atual;
								pthread_mutex_unlock(&mutex_jogo);
								break;
							} else if (gresposta[0] == RSP_SIM) {
								gestado.valor_partida++;
								pthread_mutex_unlock(&mutex_jogo);
								//break;
							}
						} else {
							//Atualiza a carta mais forte
							Carta *carta_jogada = &gjogadores_cartas[gestado.jogador_atual][gindice_carta];
							if (carta_jogada->poder > gestado.carta_mais_forte.poder) {
								gestado.carta_mais_forte = *carta_jogada;
								gestado.jogador_carta_mais_forte = gestado.jogador_atual;
								gestado.empate_parcial = 0;
								gmao = gestado.jogador_atual;
							} else if (carta_jogada->poder == gestado.carta_mais_forte.poder) {
								//Em caso de empate, verifica se os jogadores não são do mesmo time.
								if (JOGADOR_TIME(gestado.jogador_carta_mais_forte) != JOGADOR_TIME(gestado.jogador_atual)) {
									gestado.carta_mais_forte = *carta_jogada;
									gestado.jogador_carta_mais_forte = gestado.jogador_atual;
									gestado.empate_parcial = 1;
									gmao = gestado.jogador_atual;
								}
							}

							//Atualiza o estado de todos os jogadores.
							pthread_mutex_lock(&mutex_broadcast);
							mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
							new_msg = MSG_TODOS;
							enviar_mensagem(&gmensagem, new_msg);
							pthread_mutex_unlock(&mutex_broadcast);

							//todo: enviar mensagem de jogada aceita.

							gturno++;
							gestado.jogador_atual = (gestado.jogador_atual + 1) % NUM_JOGADORES;

							//Verifica se todos os jogadores já jogaram (fim da rodada).
							if (gturno == NUM_JOGADORES) {
								#ifdef DEBUG
								printf("Fim de turno\n");
								#endif //DEBUG

								pthread_mutex_unlock(&mutex_jogo);
								break;
							}
							pthread_mutex_unlock(&mutex_jogo);
						}
					}

					pthread_mutex_lock(&mutex_jogo);
					gfase = FJ_FIM_RODADA;

					//Verificando se houve vencedor na rodada. (JOGO)
					gvencedor_partida = terminar_rodada(gvencedor_partida);
					#ifdef DEBUG
					printf("Vencedor da partida: %d (%d pontos)\n", gvencedor_partida, valor_partida[gestado.valor_partida]);
					#endif //DEBUG
					
					if (gvencedor_partida != -1) {
						pthread_mutex_unlock(&mutex_jogo);
						break;
					}
					pthread_mutex_unlock(&mutex_jogo);
				}

				//Atualiza o estado de todos os jogadores.
				pthread_mutex_lock(&mutex_broadcast);
				mensagem_fim_partida(&gmensagem, gvencedor_partida);
				new_msg = MSG_TODOS;
				enviar_mensagem(&gmensagem, new_msg);
				pthread_mutex_unlock(&mutex_broadcast);
			}
		}
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadores[i].thread.leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}