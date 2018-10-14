#include "conexao.h"

int criar_socket(in_addr_t endereco, in_port_t porta, int modo) {
	int retval;
	
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		handle_error(sfd, "socket");
	}

	if (modo == CONEXAO_MODO_SERVIDOR) {
		int enable = 1;
		retval = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (retval < 0) {
			handle_error(retval, "setsockopt(SO_REUSEADDR)");
		}
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(endereco);
	server_addr.sin_port = htons(porta);

	if (modo == CONEXAO_MODO_SERVIDOR) {
		retval = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if (retval == -1) {
			handle_error(retval, "bind");
		}

		retval = listen(sfd, NUM_JOGADORES);
		if (retval == -1) {
			handle_error(retval, "listen");
		}
	} else {
		retval = connect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if (retval == -1) {
			handle_error(retval, "connect");
		}
	}

	return sfd;
}
