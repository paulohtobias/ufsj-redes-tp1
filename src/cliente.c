#include "cliente.h"
#include <errno.h>

int criar_socket_cliente() {
	return criar_socket(INADDR_ANY, PORTA, CONEXAO_MODO_CLIENTE);
}

void *t_receive(void *arg) {
	int retval;
	Mensagem mensagem;
	char texto[512];

	//Jogo
	int8_t truco_id;
	char truco_nome[128];

	//GUI - Jogo
	GtkWidget *jogo_nome_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_nome_label"));
	GtkWidget *jogo_estado_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_estado_label"));
	GtkWidget *jogo_mesa_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_mesa_label"));
	GtkWidget *jogo_mensagem_servidor_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_mensagem_servidor_label"));

	//GUI - Chat
	GtkWidget *chat_textview_log = GTK_WIDGET(gtk_builder_get_object(builder, "chat_textview_log"));
	
	//Extra
	GtkTextIter iter_end;
	GtkTextBuffer *textbuffer;

	while (1) {
		#ifdef DEBUG
		printf("[jogador %d] read\n", jogador_id);
		#endif //DEBUG

		memset(&mensagem, 0, sizeof mensagem);
		retval = read(ssfd, &mensagem, sizeof mensagem);

		#ifdef DEBUG
		printf("\033[0;31m"); 
		mensagem_print(&mensagem, "CHEGOU DO SERVIDOR: ");
		printf("\033[0m");
		#endif

		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		} else if (retval == 0) {
			printf("O servidor encerrou a conexão.\n");
			exit(0);
		}

		if (retval > 0) {

			if (mensagem.tipo == SMT_ERRO) {
				uint32_t nerr;
				memcpy(&nerr, mensagem.dados, sizeof nerr);
				handle_error(ntohl(nerr), mensagem_tipo_str[SMT_ERRO]);
			}

			if (mensagem.tipo == SMT_CHAT) {
				gsize bl;
				gchar *u8buff = g_locale_to_utf8((char *) mensagem.dados, -1, NULL, &bl, NULL);
				textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_textview_log));
				gtk_text_buffer_get_end_iter(textbuffer, &iter_end);
				gtk_text_buffer_insert_markup(textbuffer, &iter_end, u8buff, bl);
			} else {
				//Atualiza o tipo da resposta.
				pthread_mutex_lock(&mutex_mensagem);
				mensagem_simples(&gmensagem_jogo, mensagem.tipo);
				pthread_mutex_unlock(&mutex_mensagem);
				
				if (mensagem.tipo == SMT_PROCESSANDO) {
					gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), mensagem_obter_texto(&mensagem, texto));
				} else if (mensagem.tipo == SMT_BEM_VINDO) {
					//Pegando o id e setando o nome.
					mensagem_obter_id(&mensagem, &jogador_id);
					sprintf(jogador_nome, jogador_nome_fmt, cores_times[jogador_id], jogador_id);

					#ifdef DEBUG
					printf("---= id: %d # Nome: %s =---\n", jogador_id, jogador_nome);
					#endif //DEBUG
					
					//Setando a barra de nome.
					sprintf(texto, "Você: %s", jogador_nome);
					gtk_label_set_markup(GTK_LABEL(jogo_nome_label), texto);

					//Exibindo a mensagem de boas vindas.
					sprintf(texto, mensagem_tipo_str[SMT_BEM_VINDO], jogador_nome);
					gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), texto);
				} else if (mensagem.tipo == SMT_JOGADA_ACEITA) {
					mensagem_obter_carta(&mensagem, &gindice_carta);
					
					gjogadores_cartas_jogadas[jogador_id][gindice_carta] = 1;
				} else if (mensagem.tipo == SMT_TRUCO) {
					mensagem_obter_truco_id(&mensagem, &truco_id);
					sprintf(truco_nome, jogador_nome_fmt, cores_times[truco_id], truco_id);
					sprintf(texto, mensagem_tipo_str[SMT_TRUCO], truco_nome);

					gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), texto);
				} else {
					gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), mensagem_tipo_str[mensagem.tipo]);

					if (mensagem.tipo == SMT_ENVIANDO_CARTAS) {
						mensagem_obter_cartas(&mensagem, gjogadores_cartas[jogador_id]);
						mesa_str_atualizar(jogador_id, gestado_jogadores);
						gtk_label_set_text(GTK_LABEL(jogo_mesa_label), mesa_str);
						printf("[C: ATUALIZANDO A MESA]\n%s\n", mesa_str);
					}
				}

				//Atualizando o estado, caso necessário.
				if (mensagem.atualizar_estado_jogo) {
					mensagem_obter_estado_jogo(&mensagem, &gestado);
					pontuacao_str_atualizar();
					gtk_label_set_markup(GTK_LABEL(jogo_estado_label), pontuacao_str);
				}
				if (mensagem.atualizar_estado_jogadores) {
					mensagem_obter_estado_jogadores(&mensagem, gestado_jogadores);
					mesa_str_atualizar(jogador_id, gestado_jogadores);
					gtk_label_set_text(GTK_LABEL(jogo_mesa_label), mesa_str);

					#ifdef DEBUG
					printf("[E: ATUALIZANDO A MESA]\n%s\n", mesa_str);
					#endif //DEBUG
				}
			}
		}
	}
}

void t_send(GtkEntry *entry, gpointer user_data) {
	pthread_mutex_lock(&mutex_mensagem);
	
	Mensagem *mensagem = user_data;
	const char *input = gtk_entry_get_text(entry);
	uint8_t tamanho_input = strlen(input);

	//Comandos vazios são ignorados.
	if (tamanho_input == 0) {
		pthread_mutex_unlock(&mutex_mensagem);
		return;
	}

	#ifdef DEBUG
	printf("Input (%d): '%s'\n", tamanho_input, input);
	#endif //DEBUG

	uint8_t msg_valida = 0;

	if (mensagem->tipo != SMT_CHAT){
		if (tamanho_input <= 2) {
			if (mensagem->tipo == SMT_SEU_TURNO) {
				int8_t indice_carta = atoi(input);

				#ifdef DEBUG
				printf("indice carta: %d\n", indice_carta);
				#endif //DEBUG

				/*
				 * As cartas são numeradas de 1 a 3. Se o índice for 0, então o jogador pediu truco.
				 * Se o índice for negativo (entre -3 e -1), significa que a carta foi jogada no monte.
				 */
				//Verifica se o índice da carta é valido.
				if ((indice_carta >= 0 && indice_carta <= NUM_CARTAS_MAO) || (indice_carta < 0 && indice_carta >= -NUM_CARTAS_MAO && gestado.mao_de_10 == -1)) {
					//O jogador só pode pedir truco caso atenda algumas condições.
					if (indice_carta != 0 || (gestado.time_truco != JOGADOR_TIME(jogador_id) && valor_partida[gestado.valor_partida] < VLR_DOZE)) {
						mensagem_definir_carta(mensagem, indice_carta);
						msg_valida = 1;
					}
				}
			} else if (mensagem->tipo == SMT_TRUCO) {
				uint8_t resposta = atoi(input);
				if (resposta < RSP_INDEFINIDO) {
					mensagem_definir_resposta(mensagem, resposta);
					msg_valida = 1;
				}
			}
		}
	} else {
		mensagem_chat(mensagem,NULL, 0);
		mensagem_definir_textof(mensagem, "%s: %s\n", jogador_nome, input);
		msg_valida = 1;

		#ifdef DEBUG
		mensagem_print(mensagem, "Mensagem do chat");
		printf("Tamanho da mensagem do chat: %lu\n", mensagem_obter_tamanho(mensagem));
		#endif //DEBUG
	}

	if (!msg_valida) {
		#ifdef DEBUG
		printf("Entrada inválida\n");
		#endif //DEBUG
		pthread_mutex_unlock(&mutex_mensagem);
		return;
	}

	int retval = write(ssfd, mensagem, mensagem_obter_tamanho(mensagem));
	
	#ifdef DEBUG
	printf("escreveu %d bytes da msg %d\n", retval, mensagem->tipo);
	#endif //DEBUG
	
	pthread_mutex_unlock(&mutex_mensagem);
	if (retval == -1) {
		handle_error(1, "thread_escrita-write");
	}

	gtk_entry_set_text(entry, "");
}

