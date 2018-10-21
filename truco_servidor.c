#include "servidor.h"
#include <time.h>

int main(int argc, char *argv[]) {
	int i, j;

	srand((unsigned) time(NULL));

	int chat_socket_fd = criar_socket_servidor();

	pthread_create(&thread_escrita, NULL, t_escrita, NULL);

	for (i = 0; i < NUM_JOGADORES; i++) {
		jogadores[i].id = -1;
	}
	
	for (i = 0; i < NUM_JOGADORES; i++) {
		//Faz conexão inicial com o cliente.
		int jsfd = s_accept(chat_socket_fd);

		jogador_init(jogadores + i, i, jsfd);
	}

	sleep(3);
	printf("\n\n");

	gvencedor_jogo = -1;
	gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);
	gfase = FJ_ENVIANDO_CARTAS;
	gmao = rand() % NUM_JOGADORES;

	Mensagem mensagem_jogo;
	
	//Iniciando o jogo.
	extern pthread_cond_t cond_init;
	pthread_cond_broadcast(&cond_init);
	
	//Loop infinito até que os jogadores fechem o jogo.
	while (1) {
		
		//While da queda. Cada iteração é um jogo.
		while (1) {

			//While do jogo: cada iteração é uma partida.
			while (1) {
				embaralhar(gbaralho, NUM_CARTAS);
				pthread_mutex_lock(&mutex_jogo);
				gfase = FJ_ENVIANDO_CARTAS;
				for (i = 0; i < NUM_JOGADORES; i++) {
					for (j = 0; j < NUM_CARTAS_MAO; j++) {
						gjogadores_cartas[i][j] = gbaralho[i * NUM_JOGADORES + j];
						gjogadores_cartas_jogadas[i][j] = 0;
					}
					carta_virar(&gestado_jogadores[i].carta_jogada);
					gestado_jogadores[i].qtd_cartas_mao = 3;
					//Avisa o jogador.
					mensagem_enviando_cartas(&mensagem_jogo, gjogadores_cartas[i]);
					enviar_mensagem(&mensagem_jogo, MSG_JOGADOR(i));
				}

				mensagem_atualizar_estado(&mensagem_jogo, NUM_JOGADORES, gestado_jogadores, 1, &gestado);
				enviar_mensagem(&mensagem_jogo, MSG_TODOS);

				pthread_mutex_unlock(&mutex_jogo);

				//While da partida: cada iteração é uma rodada (JOGO)
				//iniciar_rodada();
				while (1) {

					pthread_mutex_lock(&mutex_jogo);
					gturno = 0;
					gestado.jogador_atual = gmao;
					printf("[JOGO] Inicio do turno. Jogador atual: %d\n", gestado.jogador_atual);
					pthread_mutex_unlock(&mutex_jogo);
					//While da rodada: cada iteração é um turno.
					while (1) {
						//Marca a fase do jogo como TURNO.
						pthread_mutex_lock(&mutex_jogo);

						gfase = FJ_TURNO;
						gjogadores_ativos = JA_JOGADOR(gestado.jogador_atual);

						//pthread_mutex_lock(&mutex_broadcast);
						
						mensagem_seu_turno(&mensagem_jogo);
						enviar_mensagem(&mensagem_jogo, MSG_JOGADOR(gestado.jogador_atual));
						
						//pthread_mutex_unlock(&mutex_broadcast);
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
							mensagem_truco(&mensagem_jogo);
							gresposta[0] = gresposta[1] = RSP_INDEFINIDO;
							uint8_t time_adversario = (JOGADOR_TIME(gestado.jogador_atual) == 0) ? MSG_TIME1 : MSG_TIME2;
							gjogadores_ativos = time_adversario;
							enviar_mensagem(&mensagem_jogo, time_adversario);
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
							mensagem_atualizar_estado(&mensagem_jogo, 1, &gestado_jogadores[gestado.jogador_atual], 0, NULL);
							enviar_mensagem(&mensagem_jogo, MSG_TODOS);

							//todo: enviar mensagem de jogada aceita.

							gturno++;
							gestado.jogador_atual = (gestado.jogador_atual + 1) % NUM_JOGADORES;

							//Verifica se todos os jogadores já jogaram (fim da rodada).
							if (gturno == NUM_JOGADORES) {
								#ifdef DEBUG
								printf("Fim de turno\n");
								#endif //DEBUG

								pthread_mutex_unlock(&mutex_jogo);
								gturno = 0;//break;
							}
							pthread_mutex_unlock(&mutex_jogo);
						}
					}

					gfase = FJ_FIM_RODADA;

					//Verificando se houve vencedor na rodada. (JOGO)
					gvencedor_partida = terminar_rodada(gvencedor_partida);
					if (gvencedor_partida != -1) {
						break;
					}
				}

				//Envia mensagem para os jogadores (BROADCAST)
				mensagem_fim_rodada(&gmensagem);
				new_msg = MSG_TODOS;
				pthread_cond_signal(&cond_new_msg);
			}
		}
	}

	for (i = 0; i < NUM_JOGADORES; i++) {
		pthread_join(jogadores[i].thread.leitura, NULL);
	}
	pthread_join(thread_escrita, NULL);

	return 0;
}