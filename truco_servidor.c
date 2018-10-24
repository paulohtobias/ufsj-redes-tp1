#include "servidor.h"
#include <time.h>

int main(int argc, char *argv[]) {
	int i, j;

	unsigned int semente = 1540161191;
	if (argc > 1) {
		semente = (unsigned) atoi(argv[1]);
	} else {
		semente = (unsigned) time(NULL);
	}
	srand(semente);

	#ifdef DEBUG
	printf("SEMENTE: %d\n", semente);
	#endif //DEBUG

	int servidor_socket_fd = criar_socket_servidor();

	for (i = 0; i < NUM_JOGADORES; i++) {
		jogadores[i].id = -1;
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(servidor_socket_fd);

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
	
	//Loop infinito até que os jogadores fechem o jogo.
	for (gqueda = 0; ; gqueda++) {

		#ifdef DEBUG
		printf("Queda %d\n", gqueda);
		#endif //DEBUG
		
		//For da queda. Cada iteração é um jogo (melhor de 3).
		for (gjogo = 0; gjogo < 3; gjogo++) {

			//For do jogo: cada iteração é uma partida (até alguém somar 12+ pontos).
			for (gpartida = 0; ; gpartida++) {
				pthread_mutex_lock(&mutex_jogo);
				iniciar_partida();
				
				//Distribui as cartas a todos os jogadores.
				for (i = 0; i < NUM_JOGADORES; i++) {
					for (j = 0; j < NUM_CARTAS_MAO; j++) {
						gjogadores_cartas[i][j] = gbaralho[i * NUM_JOGADORES + j];
						gjogadores_cartas_jogadas[i][j] = 0;
					}
					carta_virar(&gestado_jogadores[i].carta_jogada);
					gestado_jogadores[i].qtd_cartas_mao = 3;

					//Avisa o jogador.
					servidor_mensagem_enviar_cartas(i);
				}

				//Se estiver na mão de 10, é preciso saber se os jogadores vão querer jogar.
				if (!MAO_DE_FERRO) {
					for (i = 0; i < 2; i++) {
						//Se o time está na mão de 10 e o adversário não tem 0 ponto.
						if ((gestado.mao_de_10 == 1 << i) && gestado.pontos[!i] > 0) {
							gfase = FJ_MAO_DE_10;
							gjogadores_ativos = i ? JA_TIME2 : JA_TIME1;

							//Avisa o time.
							gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
							servidor_mensagem_mao_de_10(i);
							pthread_mutex_unlock(&mutex_jogo);


							pthread_mutex_lock(&mutex_jogo);
							//Aguarda a resposta.
							esperar_resposta(RSP_SIM);

							//Repostas foram dadas.
							if (gresposta[0] == RSP_NAO) {
								#if defined DEBUG || LOG
								printf("O time %d correu na mão de 10...\n", i);
								#endif //DEBUG

								//Marca o time adversário como vencedor e volta a pontuação pra 2 pontos.
								gvencedor_partida = !i;
								gestado.valor_partida = 0;
							}
							break;
							pthread_mutex_unlock(&mutex_jogo);
						}
					}
				}
				pthread_mutex_unlock(&mutex_jogo);

				//Atualiza o estado de todos os jogadores.
				servidor_mensagem_atualizar_estado(NULL);

				//For da partida: cada iteração é uma rodada
				for (grodada = 0; grodada < 3; grodada++) {
					if (gvencedor_partida == -1) {
						pthread_mutex_lock(&mutex_jogo);
						gturno = 0;
						iniciar_rodada();

						//Atualiza o estado de todos os jogadores.
						servidor_mensagem_atualizar_estado(NULL);
						
						#if defined DEBUG || LOG
						printf("Inicio do turno. Jogador atual: %d\n", gestado.jogador_atual);
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
							servidor_mensagem_seu_turno();

							//Avisa para os outros jogadores quem está jogando.
							servidor_mensagem_aguardar_turno();
							pthread_mutex_unlock(&mutex_jogo);


							pthread_mutex_lock(&mutex_jogo);
							//Aguarda a jogada.
							while (gfase != FJ_FIM_TURNO && gfase != FJ_PEDIU_TRUCO) {
								pthread_cond_wait(&cond_jogo, &mutex_jogo);
							}
							//Jogada foi feita.

							//Se o jogador pediu truco|seis|nove|doze na sua vez.
							if (gfase == FJ_PEDIU_TRUCO) {
								if (gestado.mao_de_10) {
									#ifdef DEBUG
									printf("%d pediu truco na mao de 10\n", gestado.jogador_atual);
									#endif //DEBUG
									
									gvencedor_partida = JOGADOR_TIME_ADV(gestado.jogador_atual);
									pthread_mutex_unlock(&mutex_jogo);
									break;
								}
								pthread_mutex_unlock(&mutex_jogo);
								
								RESPOSTA resposta = avisar_truco(gestado.jogador_atual);
								if (resposta == RSP_NAO) {
									break;
								} else if (resposta == RSP_SIM) {
									continue;
								}
							} else { //Se cair aqui foi uma jogada normal.
								//Atualiza a carta mais forte
								Carta *carta_jogada = &gjogadores_cartas[gestado.jogador_atual][gindice_carta];
								if (carta_jogada->poder >= gestado.carta_mais_forte.poder) {
									//Em caso de empate, verifica se os jogadores não são do mesmo time.
									if (carta_jogada->poder == gestado.carta_mais_forte.poder) {
										if (gestado.empate_parcial || JOGADOR_TIME(gestado.jogador_carta_mais_forte) != JOGADOR_TIME(gestado.jogador_atual)) {
											gestado.empate_parcial = 1;
										}
									} else {
										gestado.empate_parcial = 0;
									}
									
									//Atualiza a carta e o jogador mais forte.
									gestado.carta_mais_forte = *carta_jogada;
									gestado.jogador_carta_mais_forte = gestado.jogador_atual;
									gmao = gestado.jogador_atual;
								}

								carta_esvaziar(carta_jogada);

								//Informa ao jogador que sua jogada foi aceita.
								servidor_mensagem_jogada_aceita();

								//Atualiza o estado de todos os jogadores.
								servidor_mensagem_atualizar_estado(NULL);

								gturno++;
								gestado.jogador_atual = (gestado.jogador_atual + 1) % NUM_JOGADORES;

								//Verifica se todos os jogadores já jogaram (fim da rodada).
								if (gturno == NUM_JOGADORES) {
									#if defined DEBUG || LOG
									printf("Fim de turno.\n");
									#endif //DEBUG

									pthread_mutex_unlock(&mutex_jogo);
									break;
								}
								pthread_mutex_unlock(&mutex_jogo);
							}
						}
					}

					pthread_mutex_lock(&mutex_jogo);
					//Verifica se deu empate. Se for na primeira rodada ou se o desempate da segunda
					//terminar empatado, então os jogadores devem mostrar sua maior carta.
					if (gvencedor_partida == -1 && gestado.empate_parcial && (grodada == 0 || gestado.empate)) {
						gestado.empate = 1;
						
						//Informa que houve empate.
						servidor_mensagem_empate();

						#if defined DEBUG || LOG
						printf("Emapte na rodada %d\n", grodada);
						#endif //DEBUG
					} else {
						gfase = FJ_FIM_RODADA;

						if (grodada == 0) {
							gvencedor_primeira_rodada = JOGADOR_TIME(gestado.jogador_carta_mais_forte);

							#if defined DEBUG || LOG
							printf("O time %d fez a primeira\n", gvencedor_primeira_rodada);
							#endif //DEBUG
						} else if (gestado.empate_parcial) {
							// Se houve empate mas não foi na primeira rodada, então o vencedor é quem fez a primeira.
							gvencedor_partida = gvencedor_primeira_rodada;

							#if defined DEBUG || LOG
							printf("Emapte na %d rodada. O time %d ganha por ter feito a primeira\n", grodada, gvencedor_primeira_rodada);
							#endif //DEBUG
						} else if (gestado.empate) {
							gvencedor_partida = JOGADOR_TIME(gestado.jogador_carta_mais_forte);

							#if defined DEBUG || LOG
							printf("O time %d ganhou o desempate\n", gvencedor_partida);
							#endif //DEBUG
						}

						//Verificando se houve vencedor na rodada.
						gvencedor_partida = terminar_rodada(gvencedor_partida);

						#if defined DEBUG || LOG
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
				servidor_mensagem_atualizar_estado(NULL);


				pthread_mutex_lock(&mutex_jogo);
				//Verificando se houve vencedor na partida.
				gvencedor_jogo = terminar_partida(gvencedor_partida);

				#if defined DEBUG || LOG
				printf("Vencedor do jogo: %d\n", gvencedor_jogo);
				#endif //DEBUG

				if (gvencedor_jogo >= 0) {
					pthread_mutex_unlock(&mutex_jogo);
					break;
				}
				pthread_mutex_unlock(&mutex_jogo);
			}

			//Verificando se houve vencedor na queda.
			pthread_mutex_lock(&mutex_jogo);
			gvencedor_queda = terminar_jogo();

			#if defined DEBUG || LOG
			printf("Vencedor da queda: %d\n", gvencedor_queda);
			#endif //DEBUG

			if (gvencedor_queda > -1) {
				//Pergunta aos jogadores se vão querer continuar jogando.
				gfase = FJ_FIM_QUEDA;
				gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
				gjogadores_ativos = JA_TODOS;
				servidor_mensagem_fim_queda();
				pthread_mutex_unlock(&mutex_jogo);

				pthread_mutex_lock(&mutex_jogo);
				//Aguarda a resposta.
				esperar_resposta(RSP_SIM);

				//Repostas foram dadas.
				if (gresposta[0] == RSP_NAO) {
					#if defined DEBUG || LOG
					printf("Fim de queda. Encerrando...\n");
					#endif //DEBUG
					
					exit(0);
				} else if (gresposta[0] == RSP_SIM) {
					#if defined DEBUG || LOG
					printf("Fim de queda. Recomeçando\n");
					#endif //DEBUG

					pthread_mutex_unlock(&mutex_jogo);
				}
			}
			pthread_mutex_unlock(&mutex_jogo);
		}
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadores[i].thread_leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}