# Chat?

- **SERVIDOR: Criar um socket pro chat e 8 threads extras (leitura/escrita p/ cada jogador).**
	- **Thread-leitura loop:**
		- `read(meu_cliente_sfd, meu_buffer);`
		- `strcpy(meu_buffer_copia, meu_buffer);`
		- `wake(minha_thread_escrita);`
	- **Thread-escrita loop:**
		- `wait_signal(minha_thread_leitura);`
		- `mutex_lock(mutex_broadcast);`
		- `for (jogador in jogadores):`
			- `write(jogador.csfd, meu_buffer_copia);`
		- `mutex_unlock(mutex_broadcast);`
- **CLIENTE: Duas threads p/ escrita/leitura.**
	- **Thread-leitura loop:**
		- `read(ssfd, chat_buff);`
		- `printf(chat_buff);`
	- **Thread-escrita loop:**
		- `scanf("%s", msg_buff);`
		- `write(ssfd, msg_buff);`