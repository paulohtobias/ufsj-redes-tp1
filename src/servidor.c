#include "servidor.h"

mutex_broadcast = PTHREAD_MUTEX_INITIALIZER;
mutex_new_msg = PTHREAD_MUTEX_INITIALIZER;
cond_new_msg = PTHREAD_COND_INITIALIZER;
new_msg = 0;

void jogador_init(Jogador *jogador, uint8_t id, int sfd) {
	jogador->id = id;
	jogador->socket_fd = sfd;

	jogador->thread.new_msg = 0;
	pthread_mutex_init(&jogador->thread.new_msg_mutex, NULL);
	pthread_cond_init(&jogador->thread.new_msg_cond, NULL);

	pthread_create(&jogador->thread.leitura, NULL, t_leitura, jogador);
	//pthread_create(&jogador->thread.escrita, NULL, t_escrita, jogador);
}

void *t_leitura(void *args) {
	Jogador *jogador = args;

	int retval;
	char jogador_name[128];
	Mensagem mensagem;

	//Setando o nome do jogador
	sprintf(
		jogador_name,
		"<span font_weight='bold' color='%s'>client %d: </span>",
		cores_times[jogador->id], jogador->id
	);

	// Loop principal
	while (1) {
		#ifdef DEBUG
		printf("client %d: thread_leitura-read\n", jogador->id);
		#endif //DEBUG

		//Espera comandos do jogador.
		memset(&mensagem, 0, sizeof mensagem);
		retval = read(jogador->socket_fd, &mensagem, sizeof mensagem);
		if (retval == -1) {
			handle_error(1, "thread_leitura-read");
		} else if (retval == 0) {
			printf("O cliente %d encerrou a conexão.\n", jogador->id);
			//todo: return NULL;
			exit(0);
		}

		#ifdef DEBUG
		printf("Mensagem [%2d]: '%s'\n", mensagem.tipo, mensagem_tipo_str[mensagem.tipo]);
		#endif //DEBUG

		if (mensagem.tipo == SMT_CHAT) {
			pthread_mutex_lock(&mutex_broadcast);
			gmensagem.tipo = SMT_CHAT;
			snprintf(gmensagem.dados, BUFF_SIZE, "%s%s\n", jogador_name, mensagem.dados);
			gmensagem.tamanho_dados = strlen(gmensagem.dados) + 1;
			new_msg = MSG_TODOS;
			pthread_cond_signal(&cond_new_msg);
			pthread_mutex_unlock(&mutex_broadcast);
		}

		if (jogador->thread.new_msg) {
		}
	}
}

void *t_escrita(void *args) {
	int i, retval;

	while (1) {
		//Espera uma nova mensagem chegar.
		pthread_mutex_lock(mutex_new_msg);
		while (!new_msg) {
			pthread_cond_wait(&cond_new_msg, &mutex_new_msg);
		}
		pthread_mutex_unlock(&mutex_new_msg);

		//Envia a mensagem aos clientes.
		pthread_mutex_lock(&mutex_broadcast);
		for (i = 0; i < NUM_JOGADORES; i++) {
			if ((1 << i) & new_msg) {
				write(jogadres[i].socket_fd, &gmensagem, sizeof gmensagem);
			}
		}
		new_msg = MSG_NINGUEM;
		pthread_mutex_unlock(&mutex_broadcast);
	}
}
