#include "cliente.h"

int main(int argc, char *argv[]) {
	Mensagem mensagem;
	mensagem.tipo = SMT_CHAT;

	//GUI
	gtk_init(&argc, &argv);
	builder = gtk_builder_new_from_file("gui.glade");
	
	GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	// Reposicionando a janela.
	if (argc >= 3) {
		gtk_window_move(GTK_WINDOW(window), atoi(argv[1]), atoi(argv[2]));
	}

	GtkWidget *chat_entry = GTK_WIDGET(gtk_builder_get_object(builder, "chat_entry"));
	gtk_entry_set_max_length(GTK_ENTRY(chat_entry), BUFF_SIZE -1);
	g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(t_send), &mensagem);
	
	
	//Socket
	ssfd = criar_socket_cliente();

	printf("[Client] connected to server.\n");

	pthread_create(&thread, NULL, t_receive, NULL);

	//todo: apagar o keep_above
	gtk_window_set_keep_above(GTK_WINDOW(window), 1);
	gtk_window_present(GTK_WINDOW(window));
	gtk_main();

	return 0;
}
