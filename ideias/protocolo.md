# **Protocolo de Mensagens**

- **Tipos de mensagem:**
	- **Servidor -> Cliente:**
		- "Exiba a mensagem a seguir na sua tela". (talvez o cliente escreve a mensagem a partir das mensagens que chegarem?)
		- "Chat".
		- "Pontuação e estado do jogo".
		- "Vou te enviar as cartas. Aceita|Quiema|Corre?".
		- "Suas 3 cartas são".
		- "Sua vez de jogar".
		- "O jogador X jogou a carta Y". [REMOVER?]
		- "O adversário X pediu Truco|Seis|Nove|Doze. Me envie sua resposta: Aceito|Corro|Subo".
		- "Houve empate. Me mostre sua maior carta".
		- "Houve empate. ".

	- **Cliente -> Servidor:**
		- BitMask tipo: ()
