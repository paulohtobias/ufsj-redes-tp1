#ifndef CLIENTE_H
#define CLIENTE_H

#include "conexao.h"
#include <gtk/gtk.h>

Carta cartas[NUM_CARTAS_MAO];

int ssfd;
pthread_t thread;

GtkBuilder *builder;

int criar_socket_cliente();

///Lê o log do chat do servidor e exibe na tela.
void *t_receive(void *);

void t_send(GtkEntry *entry, gpointer user_data);

#endif //CLIENTE_H