#include "servidor.h"
#include <time.h>

int main(int argc, char *argv[]) {
	int i, j;

	unsigned int semente = (unsigned) time(NULL);
	srand(semente);

	#ifdef DEBUG
	printf("SEMENTE: %d\n", semente);
	#endif //DEBUG

	int chat_socket_fd = criar_socket_servidor();

	for (i = 0; i < NUM_JOGADORES; i++) {
		jogadores[i].id = -1;
	}
	
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(chat_socket_fd);

		jogador_init(jogadores + i, i, jsfd);
	}

	pthread_mutex_lock(&mutex_jogo);
	gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
	gjogador_baralho = rand() % NUM_JOGADORES;
	pthread_mutex_unlock(&mutex_jogo);
	
	//Iniciando o jogo.
	pthread_mutex_lock(&mutex_init);
	while (num_jogadores < NUM_JOGADORES) {
		pthread_cond_wait(&cond_init, &mutex_init);
	}
	pthread_mutex_unlock(&mutex_init);

	int queda, jogo, partida, rodada, turno;
	
	//Loop infinito até que os jogadores fechem o jogo.
	for (queda = 0; ; queda++) {
		
		//For da queda. Cada iteração é um jogo (melhor de 3).
		for (jogo = 0; jogo < 3; jogo++) {

			//For do jogo: cada iteração é uma partida (até alguém somar 12+ pontos).
			for (partida = 0; ; partida++) {
				pthread_mutex_lock(&mutex_jogo);
				gestado.empate = 0;
				gvencedor_partida = -1;
				int rodadas = 0;
				gmao = gjogador_baralho;
				gjogador_baralho = (gjogador_baralho + 1) % NUM_JOGADORES;

				//Embaralha as cartas.
				embaralhar(gbaralho, NUM_CARTAS, 10);
				
				//Muda a fase do jogo.
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

				//For da partida: cada iteração é uma rodada
				//iniciar_rodada();
				for (rodada = 0; rodada < 3; rodada++) {
					pthread_mutex_lock(&mutex_jogo);
					turno = 0;
					gestado.empate_parcial = 0;
					gestado.jogador_carta_mais_forte = -1;
					carta_virar(&gestado.carta_mais_forte);
					gestado.jogador_atual = gmao;
					
					for (i = 0; i < NUM_JOGADORES; i++) {
						carta_esvaziar(&gestado_jogadores[i].carta_jogada);
					}

					//Atualiza o estado de todos os jogadores.
					pthread_mutex_lock(&mutex_broadcast);
					mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
					new_msg = MSG_TODOS;
					enviar_mensagem(&gmensagem, new_msg);
					pthread_mutex_unlock(&mutex_broadcast);
					
					#ifdef DEBUG
					printf("[JOGO] Inicio do turno.\n$$$ Jogador atual: %d\n", gestado.jogador_atual);
					#endif //DEBUG
					
					pthread_mutex_unlock(&mutex_jogo);
					//While da rodada: cada iteração é um turno (uma jogada individual de um jogador).
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

						//Avisa para os outros jogadores quem está jogando.
						pthread_mutex_lock(&mutex_broadcast);
						mensagem_processando(&gmensagem, "");
						mensagem_definir_textof(&gmensagem, "Aguardando a jogada do Jogador %d", gestado.jogador_atual);
						new_msg = MSG_TODOS - MSG_JOGADOR(gestado.jogador_atual);
						enviar_mensagem(&gmensagem, new_msg);
						pthread_mutex_unlock(&mutex_broadcast);
						pthread_mutex_unlock(&mutex_jogo);


						pthread_mutex_lock(&mutex_jogo);
						//Aguarda a jogada.
						while (gfase != FJ_FIM_TURNO && gfase != FJ_PEDIU_TRUCO) {
							pthread_cond_wait(&cond_jogo, &mutex_jogo);
						}
						//Jogada foi feita.

						//Se o jogador pediu truco|seis|nove|doze na sua vez.
						if (gfase == FJ_PEDIU_TRUCO) {
							if (gestado.mao_de_10 != -1) {
								#ifdef DEBUG
								printf("%d pediu truco na mao de 10\n", gestado.jogador_atual);
								#endif //DEBUG
								
								gvencedor_partida = !JOGADOR_TIME(gestado.jogador_atual);
								pthread_mutex_unlock(&mutex_jogo);
								break;
							}

							//todo: criar uma função avisar_truco(jogador_id) no servidor
							//Avisa para todos que o jogador pediu truco.
							pthread_mutex_lock(&mutex_broadcast);
							mensagem_processando(&gmensagem, "Seu time pediu truco|seis|nove|doze. Aguardando resposta do time adversário.");
							new_msg = MSG_TIME(gestado.jogador_atual);
							enviar_mensagem(&gmensagem, new_msg);
							pthread_mutex_unlock(&mutex_broadcast);							
							
							//Avisa ao outro time que o jogador pediu truco e espera pela reposta.
							pthread_mutex_lock(&mutex_broadcast);
							gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
							mensagem_truco(&gmensagem, gestado.jogador_atual);
							uint8_t time_adversario = MSG_TIME_ADV(gestado.jogador_atual);
							gjogadores_ativos = time_adversario;
							enviar_mensagem(&gmensagem, time_adversario);
							pthread_mutex_unlock(&mutex_broadcast);
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
								printf("o time %d correu do truco|seis|nove|doze\n", !gestado.jogador_atual);
								#endif //DEBUG
								
								gvencedor_partida = JOGADOR_TIME(gestado.jogador_atual);
								pthread_mutex_unlock(&mutex_jogo);
								break;
							} else if (gresposta[0] == RSP_SIM) {
								gestado.valor_partida++;

								//Atualização geral.
								pthread_mutex_lock(&mutex_broadcast);
								mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
								mensagem_definir_textof(&gmensagem, "Truco|Seis|Nove|Doze Aceito.");
								new_msg = MSG_TODOS;
								enviar_mensagem(&gmensagem, new_msg);
								pthread_mutex_unlock(&mutex_broadcast);

								#ifdef DEBUG
								printf("o time %d aceitou o truco|seis|nove|doze\n", !gestado.jogador_atual);
								#endif //DEBUG

								pthread_mutex_unlock(&mutex_jogo);
								continue;
							} else { //Se cair aqui, então a repoosta foi pra aumentar a aposta.
								//todo: criar uma função avisar_truco(jogador_id) no servidor
								//Avisa para o time que o jogador pediu truco.
								pthread_mutex_lock(&mutex_broadcast);
								mensagem_processando(&gmensagem, "Seu time pediu truco|seis|nove|doze. Aguardando resposta do time adversário.");
								uint8_t time_adversario = MSG_TIME_ADV(gestado.jogador_atual);
								gjogadores_ativos = time_adversario;
								enviar_mensagem(&gmensagem, time_adversario);
								pthread_mutex_unlock(&mutex_broadcast);							
								
								//Avisa ao outro time que o jogador pediu truco e espera pela reposta.
								pthread_mutex_lock(&mutex_broadcast);
								gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
								mensagem_truco(&gmensagem, gestado.jogador_atual);
								new_msg = MSG_TIME(gestado.jogador_atual);
								enviar_mensagem(&gmensagem, new_msg);
								pthread_mutex_unlock(&mutex_broadcast);
								pthread_mutex_unlock(&mutex_jogo);

								#ifdef DEBUG
								printf("TO-DO: o time %d aumentou!! truco|seis|nove|doze\n", !gestado.jogador_atual);
								#endif //DEBUG
							}
						} else { //Se cair aqui foi uma jogada normal.
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

							carta_esvaziar(carta_jogada);

							//Informa ao jogador que sua jogada foi aceita.
							pthread_mutex_lock(&mutex_broadcast);
							mensagem_jogada_aceita(&gmensagem, &gestado, gindice_carta);
							new_msg = MSG_JOGADOR(gestado.jogador_atual);
							enviar_mensagem(&gmensagem, new_msg);
							pthread_mutex_unlock(&mutex_broadcast);

							//Atualiza o estado de todos os jogadores.
							pthread_mutex_lock(&mutex_broadcast);
							mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
							new_msg = MSG_TODOS;
							enviar_mensagem(&gmensagem, new_msg);
							pthread_mutex_unlock(&mutex_broadcast);

							turno++;
							gestado.jogador_atual = (gestado.jogador_atual + 1) % NUM_JOGADORES;

							//Verifica se todos os jogadores já jogaram (fim da rodada).
							if (turno == NUM_JOGADORES) {
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
					//Verifica se deu empate. Se for na primeira rodada ou se o desempate da segunda
					//terminar empatado, então os jogadores devem mostrar sua maior carta.
					if (gestado.empate_parcial && (rodada == 0 || gestado.empate)) {
						gestado.empate = 1;
						
						//Informa que houve empate.
						pthread_mutex_lock(&mutex_broadcast);
						mensagem_empate(&gmensagem);
						new_msg = MSG_TODOS;
						enviar_mensagem(&gmensagem, new_msg);
						pthread_mutex_unlock(&mutex_broadcast);

						#ifdef DEBUG
						printf("Emapte na primeira rodada (e talvez na segunda também)\n");
						#endif //DEBUG
					} else {
						gfase = FJ_FIM_RODADA;

						if (rodada == 0) {
							gvencedor_primeira_rodada = JOGADOR_TIME(gestado.jogador_carta_mais_forte);

							#ifdef DEBUG
							printf("O time %d fez a primeira\n", gvencedor_primeira_rodada);
							#endif //DEBUG
						} else if (gestado.empate_parcial) {
							// Se houve empate mas não foi na primeira rodada, então o vencedor é quem fez a primeira.
							gvencedor_partida = gvencedor_primeira_rodada;

							#ifdef DEBUG
							printf("Emapte na %d rodada. O time %d ganha por ter feito a primeira\n", rodadas, gvencedor_primeira_rodada);
							#endif //DEBUG
						} else if (gestado.empate) {
							gvencedor_partida = JOGADOR_TIME(gestado.jogador_carta_mais_forte);

							#ifdef DEBUG
							printf("O time %d ganhou o desempate\n", gvencedor_partida);
							#endif //DEBUG
						}

						//Verificando se houve vencedor na rodada.
						gvencedor_partida = terminar_rodada(gvencedor_partida);

						#ifdef DEBUG
						printf("Vencedor da partida: %d (%d pontos)\n", gvencedor_partida, valor_partida[gestado.valor_partida]);
						#endif //DEBUG

						if (gvencedor_partida >= 0) {
							pthread_mutex_unlock(&mutex_jogo);
							break;
						}
					}
					pthread_mutex_unlock(&mutex_jogo);
				}

				//Atualiza o estado de todos os jogadores.
				pthread_mutex_lock(&mutex_broadcast);
				mensagem_atualizar_estado(&gmensagem, &gestado, gestado_jogadores);
				new_msg = MSG_TODOS;
				enviar_mensagem(&gmensagem, new_msg);
				pthread_mutex_unlock(&mutex_broadcast);


				pthread_mutex_lock(&mutex_jogo);
				//Verificando se houve vencedor na partida.
				gvencedor_jogo = terminar_partida(gvencedor_partida);

				#ifdef DEBUG
				printf("Vencedor do jogo: %d\n", gvencedor_jogo);
				#endif //DEBUG

				if (gvencedor_jogo >= 0) {
					pthread_mutex_unlock(&mutex_jogo);
					exit(0);
					break;
				}
				pthread_mutex_unlock(&mutex_jogo);
			}
		}
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadores[i].thread.leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}