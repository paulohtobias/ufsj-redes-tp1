#include "cliente.h"

int criar_socket_cliente() {
	return criar_socket(INADDR_ANY, PORTA, CONEXAO_MODO_CLIENTE);
}

void *t_receive(void *arg) {
	int retval;
	Mensagem mensagem;

	//Jogo
	EstadoJogo estado_jogo;
	EstadoJogador estado_jogadores[NUM_JOGADORES];

	//GUI - Jogo
	GtkWidget *jogo_estado_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_estado_label"));
	GtkWidget *jogo_textview_log = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_textview_log"));
	GtkWidget *jogo_mensagem_servidor_label = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_mensagem_servidor_label"));

	//GUI - Chat
	GtkWidget *chat_textview_log = GTK_WIDGET(gtk_builder_get_object(builder, "chat_textview_log"));
	
	//Extra
	GtkTextIter iter_end;
	GtkTextBuffer *textbuffer;

	while (1) {
		retval = read(ssfd, &mensagem, sizeof mensagem);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		}

		if (retval > 0) {
			#ifdef DEBUG
			mensagem_print(&mensagem, "CHEGOU DO SERVIDOR: ");
			#endif

			//Atualizando o estado, caso necessário.
			if (mensagem.atualizar_pontuacao) {
				mensagem_obter_pontuacao(&mensagem, &estado_jogo);
				pontuacao_str_atualizar(&estado_jogo);
				gtk_label_set_markup(GTK_LABEL(jogo_estado_label), pontuacao_str);
			}
			if (mensagem.estados > 0) {
				mensagem_obter_estado_jogadores(&mensagem, estado_jogadores);
			}
			
			if (mensagem.tipo == SMT_BEM_VINDO) {
				gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), mensagem_obter_texto(&mensagem));
			}
			else if (mensagem.tipo == SMT_PROCESSANDO) {
				gtk_label_set_markup(GTK_LABEL(jogo_mensagem_servidor_label), mensagem_obter_texto(&mensagem));
			}
			else if (mensagem.tipo == SMT_CHAT) {
				gsize bl;
				gchar *u8buff = g_locale_to_utf8((char *) mensagem.dados, -1, NULL, &bl, NULL);
				textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_textview_log));
				gtk_text_buffer_get_end_iter(textbuffer, &iter_end);
				gtk_text_buffer_insert_markup(textbuffer, &iter_end, u8buff, bl);
			}
		}
	}
}

void t_send(GtkEntry *entry, gpointer user_data) {
	Mensagem mensagem;
	memcpy(&mensagem, user_data, sizeof mensagem);

	strncpy((char *) mensagem.dados, gtk_entry_get_text(entry), BUFF_SIZE);
	mensagem.tamanho_dados = strlen((char *) mensagem.dados) + 1;
	int retval = write(ssfd, &mensagem, mensagem_obter_tamanho(&mensagem));
	if (retval == -1) {
		handle_error(1, "thread_escrita-write");
	}

	#ifdef DEBUG
	printf("lido: <%d> '%s'\n", retval, (char *) mensagem.dados);
	#endif

	gtk_entry_set_text(entry, "");
}

