A função del não devolve o esperado em certos casos.
A função getkeys faz a chamada ao servidor, mas o seu resultado não é tratado devido a problemas.
Alguns memory leaks relativos à libertação de espaço da mensagem.

Fase 4 - As conexões são todas realizadas com múltiplos clientes e os dois servidores.
	 No entanto o fecho do primário quando já existe um backup impossibilita a comunicação cliente-servidor.
	 Todos os outros casos parecem correr sem problemas.
