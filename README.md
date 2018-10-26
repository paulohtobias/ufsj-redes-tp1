# Primeiro Trabalho Prático de Redes ♥♦♣♠

## João Vitor Gonçalves e Paulo Henrique Tobias

- **Compilar:**
	- Para compilar somente o módulo servidor: `make servidor`
	- Para compilar somente o módulo cliente: `make cliente`
	- Para compilar ambos: `make`

- **Executar:**
	- Para executar o servidor: `./servidor`
	- Para abrir uma instância do cliente: `./cliente`
	- A flag `-e` pode ser ativada, juntamente com o endereço ip do servidor, caso este esteja sendo executado em outra máquina.
	- A flag `-l` pode ser ativada em ambos para que os logs sejam exibidos na tela.

- **Como jogar:**
	- Existem dois times: azul e vermelho. O time azul é composto pelos jogadores 0 e 2 enquanto o time vermelho é composto pelos jogadores 1 e 3.
	- Com o servidor em execução, execute o arquivo `cliente` e espere por mais 3 jogadores.
	- Quando for seu turno, você poderá enviar uma carta usando o índice dela na sua mão. Este índice pode ser `1`, `2` ou `3`. Caso queira jogar a carta encoberta, utilize o índice da carta com um sinal de menos na frente, e.g. `-2`. Para trucar, utilize `0`.
	- Quando um jogador adversário pedir truco, você deve responder se deseja recusar, aceitar ou aumentar a aposta. O jogo não prosseguirá até que você e seu parceiro enviem a mesma resposta (vocês podem usar o chat para se comunicar e chegar em um acordo).
	- Se seu time estiver na Mão de 10, você, após receber suas cartas, poderá decidir se quer participar ou não. O jogo não prosseguirá até que você e seu parceiro enviem a mesma resposta.
	- Ao final da queda, você ou seu parceiro deverão decidir se vão continuar jogando. Os jogadores do outro time também devem responder. As respostas dos dois times (note que respostas de jogadores do mesmo time se sobrescrevem) devem ser a mesma.
