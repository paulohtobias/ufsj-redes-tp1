#ifndef CONEXAO_H
#define CONEXAO_H

#include "protocolo.h"

#define PORTA 2222

enum {
	CONEXAO_MODO_CLIENTE,
	CONEXAO_MODO_SERVIDOR
};

int criar_socket(in_addr_t endereco, in_port_t porta, int modo);

#endif //CONEXAO_H
