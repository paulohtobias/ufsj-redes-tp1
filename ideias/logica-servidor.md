# **Lógica do Servidor**

- **Servidor espera os 4 jogadores entrarem.**

- **Faz toda a preparação inicial do jogo:**
	- Decide os times (pode ser feito para os jogadores escolherem)
	- Zera a pontuação.
	- Abre threads p/ escrita e leitura com todos os jogadores.
	- cada cliente tem um bool p/ saber se está na sua vez.

- **Loop principal:**
	- Informa a pontuação atual a todos os jogadores.
	- Embaralha e distribui as cartas.

- **Loop principal (old):**
	- Informa a pontuação atual a todos os jogadores.
	- Embaralha e distribui as cartas.
	- Informa ao jogador atual para fazer sua jogada (`write`) e espera o retorno (`read`).
	- **Se o jogador pediu truco:**
		- 
	- Atualiza as informações do jogo.
	- Envia as informações atualizadas de jogo.


# **Lógica do CLiente**

- **Conecta-se ao servidor.**
- **espera algum comando com `read`.**