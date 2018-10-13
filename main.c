#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>

#define handle_error(cod, msg)\
	perror(msg); exit(cod);

#define PORT 2222

#define BUFF_SIZE 1024

int ssfd;
pthread_t thread;

///Lê o log do chat do servidor e exibe na tela.
void *receive_thread(void *);

GtkBuilder *builder;

void send(GtkEntry *entry, gpointer user_data);

int main(int argc, char *argv[]) {
	int i;
	int retval;

	//GUI
	gtk_init(&argc, &argv);
	builder = gtk_builder_new_from_file("gui.glade");
	
	GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *chat_entry = GTK_WIDGET(gtk_builder_get_object(builder, "chat_entry"));
	g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(send), NULL);

	char buff[BUFF_SIZE];
	
	ssfd = socket(AF_INET, SOCK_STREAM, 0);
	if (ssfd == -1) {
		handle_error(ssfd, "socket");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	retval = connect(ssfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if (retval == -1) {
		handle_error(retval, "connect");
	}

	printf("[Client] connected to server.\n");

	pthread_create(&thread, NULL, receive_thread, NULL);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}

void *receive_thread(void *arg) {
	int retval;
	char buff[BUFF_SIZE];

	GtkWidget *chat_textview_log = GTK_WIDGET(gtk_builder_get_object(builder, "chat_textview_log"));
	GtkTextIter iter_end;
	GtkTextBuffer *textbuffer;

	while (1) {
		retval = read(ssfd, buff, BUFF_SIZE);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		}

		if (retval > 0) {
			#ifdef DEBUG
			printf("CHEGOU DO SERVIDOR: '%s'\n", buff);
			#endif
			
			gsize bl;
			gchar *u8buff = g_locale_to_utf8(buff, -1, NULL, &bl, NULL);
			textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_textview_log));
			gtk_text_buffer_get_end_iter(textbuffer, &iter_end);
			gtk_text_buffer_insert_markup(textbuffer, &iter_end, u8buff, bl);
		}
	}
}

void send(GtkEntry *entry, gpointer user_data) {
	const char *command = gtk_entry_get_text(entry);
	int retval = write(ssfd, command, gtk_entry_get_text_length(entry) + 1);
	if (retval == -1) {
		handle_error(1, "thread_escrita-write");
	}

	#ifdef DEBUG
	printf("lido: <%d> '%s'\n", retval, command);
	#endif

	gtk_entry_set_text(entry, "");
}
