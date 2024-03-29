#ifndef CLIENTE_H
#define CLIENTE_H

#include "conexao.h"
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include "textscroll.h"

/* ENUMS */
enum {
	ENTRY_JOGO,
	ENTRY_CHAT
};


/* VARIÁVEIS GLOBAIS */
int8_t jogador_id;
char jogador_nome[128];
int ssfd;
pthread_t thread;
pthread_mutex_t mutex_mensagem;
pthread_mutex_t mutex_gui;
Mensagem gmensagem_jogo;
Mensagem gmensagem_chat;
GtkBuilder *builder;


/* FUNÇÕES */
int criar_socket_cliente(in_addr_t endereco);

void encerrar_programa();

void mostrar_ajuda(GtkMenuItem *menuitem, gpointer user_data);

void *t_receive(void *);

void t_send(GtkEntry *entry, gpointer user_data);

#endif //CLIENTE_H