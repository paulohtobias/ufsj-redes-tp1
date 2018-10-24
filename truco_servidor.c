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

	int queda, jogo, partida, rodada, turno;
	
	//Loop infinito até que os jogadores fechem o jogo.
	for (queda = 0; ; queda++) {

		#ifdef DEBUG
		printf("Queda %d\n", queda);
		#endif //DEBUG
		
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

				//Se estiver na mão de 10, é preciso saber se os jogadores vão querer jogar.
				if (!MAO_DE_FERRO) {
					for (i = 0; i < 2; i++) {
						if ((gestado.mao_de_10 == 1 << i) && gestado.pontos[!i] > 0) {
							gfase = FJ_MAO_DE_10;
							gjogadores_ativos = i ? JA_TIME2 : JA_TIME1;

							//Avisa o time.
							pthread_mutex_lock(&mutex_broadcast);
							gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
							mensagem_mao_de_10(&gmensagem);
							new_msg = i ? MSG_TIME2 : MSG_TIME1;
							enviar_mensagem(&gmensagem, new_msg);
							pthread_mutex_unlock(&mutex_broadcast);
							pthread_mutex_unlock(&mutex_jogo);


							pthread_mutex_lock(&mutex_jogo);
							//Aguarda a resposta.
							while (gresposta[0] > RSP_SIM || gresposta[0] != gresposta[1]) {
								#ifdef DEBUG
								printf("Aguardando o resposta mao de 10...\n");
								#endif //DEBU

								pthread_cond_wait(&cond_jogo, &mutex_jogo);
							}

							//Repostas foram dadas.
							if (gresposta[0] == RSP_NAO) {
								#if defined DEBUG || LOG
								printf("O time %d correu na mão de 10...\n", i);
								#endif //DEBUG

								gvencedor_partida = !i;

								pthread_mutex_unlock(&mutex_jogo);
							} else if (gresposta[0] == RSP_SIM) {
								#if defined DEBUG || LOG
								printf("O time %d aceitou na mão de 10...\n", i);
								#endif //DEBUG

								pthread_mutex_unlock(&mutex_jogo);
							}
						}
					}
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
				for (rodada = 0; rodada < 3 && gvencedor_partida == -1; rodada++) {
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
						mensagem_aguardar_turno(&gmensagem, gestado.jogador_atual);
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
							if (gestado.mao_de_10) {
								#ifdef DEBUG
								printf("%d pediu truco na mao de 10\n", gestado.jogador_atual);
								#endif //DEBUG
								
								gvencedor_partida = JOGADOR_TIME_ADV(gestado.jogador_atual);
								pthread_mutex_unlock(&mutex_jogo);
								break;
							}
							pthread_mutex_unlock(&mutex_jogo);
							
							RESPOSTAS resposta = avisar_truco(gestado.jogador_atual);
							if (resposta == RSP_NAO) {
								break;
							} else if (resposta == RSP_SIM) {
								continue;
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
					if (gvencedor_partida == -1 && gestado.empate_parcial && (rodada == 0 || gestado.empate)) {
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

				#if defined DEBUG || LOG
				printf("Vencedor do jogo: %d\n", gvencedor_jogo);
				#endif //DEBUG

				if (gvencedor_jogo >= 0) {
					pthread_mutex_unlock(&mutex_jogo);
					//exit(0);
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
				#if defined DEBUG || LOG
				printf("NÃO ERA PRA ENTRAR AQUI PORRA: %d\n", gvencedor_queda);
				#endif //DEBUG

				//Pergunta aos jogadores se vão querer continuar jogando.
				pthread_mutex_lock(&mutex_broadcast);
				gfase = FJ_FIM_QUEDA;
				gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
				gjogadores_ativos = JA_TODOS;
				mensagem_fim_queda(&gmensagem, gvencedor_queda);
				new_msg = MSG_TODOS;
				enviar_mensagem(&gmensagem, new_msg);
				pthread_mutex_unlock(&mutex_broadcast);
				pthread_mutex_unlock(&mutex_jogo);

				pthread_mutex_lock(&mutex_jogo);
				//Aguarda a resposta.
				while (gresposta[0] > RSP_SIM || gresposta[0] != gresposta[1]) {
					#ifdef DEBUG
					printf("Aguardando o resposta do truco...\n");
					#endif //DEBU
					
					pthread_cond_wait(&cond_jogo, &mutex_jogo);
				}

				//Repostas foram dadas.
				if (gresposta[0] == RSP_NAO) {
					#if defined DEBUG || LOG
					printf("Fim de queda. Encerrando...\n");
					#endif //DEBUG
					
					pthread_mutex_unlock(&mutex_jogo);
					break;
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

	#ifdef DEBUG
	printf("Esperando fim das threads....\n");
	#endif //DEBUG

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadores[i].thread_leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}