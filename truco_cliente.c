#include "cliente.h"

int main(int argc, char *argv[]) {
	in_addr_t endereco = INADDR_ANY;
	if (argc > 1) {
		inet_pton(AF_INET, argv[1], &endereco);
	}

	pthread_mutex_init(&mutex_mensagem, NULL);
	pthread_mutex_init(&mutex_gui, NULL);
	jogador_id = -1;

	//GUI - Builder
	gtk_init(&argc, &argv);
	builder = gtk_builder_new_from_file("gui.glade");

	//GUI - Janela
	GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(encerrar_programa), NULL);
	// Reposicionando a janela.
	if (argc >= 4) {
		gtk_window_move(GTK_WINDOW(window), atoi(argv[2]), atoi(argv[3]));
	}

	//GUI - Jogo
	GtkWidget *jogo_entry = GTK_WIDGET(gtk_builder_get_object(builder, "jogo_entry"));
	gtk_entry_set_max_length(GTK_ENTRY(jogo_entry), 2);
	g_signal_connect(G_OBJECT(jogo_entry), "activate", G_CALLBACK(t_send), &gmensagem_jogo);

	//GUI - Chat
	mensagem_simples(&gmensagem_chat, SMT_CHAT);
	GtkWidget *chat_entry = GTK_WIDGET(gtk_builder_get_object(builder, "chat_entry"));
	gtk_entry_set_max_length(GTK_ENTRY(chat_entry), BUFF_SIZE -1);
	g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(t_send), &gmensagem_chat);

	//todo: apagar o keep_above
	gtk_window_set_keep_above(GTK_WINDOW(window), 1);
	gtk_window_present(GTK_WINDOW(window));

	//Socket
	ssfd = criar_socket_cliente(endereco);

	pthread_create(&thread, NULL, t_receive, NULL);

	//Loop principal.
	while (1) {
		pthread_mutex_lock(&mutex_gui);
		g_main_context_iteration(NULL, FALSE);
		pthread_mutex_unlock(&mutex_gui);
	}

	return 0;
}
