#include "cliente.h"

int main(int argc, char *argv[]) {
	Mensagem mensagem;
	mensagem.tipo = SMT_CHAT;

	//GUI
	gtk_init(&argc, &argv);
	builder = gtk_builder_new_from_file("gui.glade");
	
	GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *chat_entry = GTK_WIDGET(gtk_builder_get_object(builder, "chat_entry"));
	gtk_entry_set_max_length(GTK_ENTRY(chat_entry), BUFF_SIZE -1);
	g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(t_send), &mensagem);
	
	
	//Socket
	ssfd = criar_socket_cliente();

	printf("[Client] connected to server.\n");

	pthread_create(&thread, NULL, t_receive, NULL);

	gdk_threads_init();
	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
