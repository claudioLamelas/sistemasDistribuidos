#include <signal.h>
#include <poll.h>

#include "message-private.h"
#include "tree_skel.h"
#include "inet.h"
#include "include/sdmessage.pb-c.h"

#define NFDESC 500
/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
 int sockfd;

int network_server_init(short port){
  struct sockaddr_in server;

  // Cria socket TCP
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
      perror("Erro ao criar socket");
      return -1;
  }

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
  // Preenche estrutura server com endereço(s) para associar (bind) à socket
  server.sin_family = AF_INET;
  server.sin_port = htons(port); // Porta TCP
  server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

  // Faz bind
  if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
      perror("Erro ao fazer bind");
      close(sockfd);
      return -1;
  }

  // Esta chamada diz ao SO que esta é uma socket para receber pedidos
  if (listen(sockfd, 0) < 0){
      perror("Erro ao executar listen");
      close(sockfd);
      return -1;
  }
  return sockfd;
}


/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket){

  int nbytes, tamanho, tamanho2;
  char* buf;
  struct message_t* msgReceived = malloc(sizeof(struct message_t));
  MessageT* msgR;

  if((nbytes = read_all(client_socket,(char*) &tamanho,sizeof(int))) < sizeof(int)){
		perror("Erro ao receber dados do cliente");
		close(client_socket);
    return NULL;
	}

  tamanho2 = ntohl(tamanho);
  buf = malloc(tamanho2);

  if((nbytes = read_all(client_socket,buf,tamanho2)) < tamanho2){
		perror("Erro ao receber dados do cliente");
		close(client_socket);
    return NULL;
	}

  msgR = message_t__unpack(NULL, tamanho2, buf);
    if (msgR == NULL) {
        fprintf(stdout, "error unpacking message\n");
        return NULL;
    }

  msgReceived->mensagem = *msgR;

  free(buf);

  return msgReceived;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg){
  int tamanho = message_t__get_packed_size(&(msg->mensagem));
  int tamanho2;
  char* buf;
  int nbytes;

  tamanho2 = htonl(tamanho);
  buf = malloc(tamanho);


  if (buf == NULL) {
        fprintf(stdout, "malloc error\n");
        return -1;
    }

  if((nbytes = write_all(client_socket,(char*)&tamanho2,sizeof(int))) != sizeof(int)){
    perror("Erro ao enviar resposta ao cliente");
    close(client_socket);
    return -1;
  }

  message_t__pack(&msg->mensagem, buf);

  if((nbytes = write_all(client_socket,buf,tamanho)) != tamanho){
    perror("Erro ao enviar resposta ao cliente");
    close(client_socket);
    return -1;
  }
  free(buf);

  return 0;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket){

  int connsockfd, nbytes, i, nfds, kfds;
  struct sockaddr_in client;
  socklen_t size_client;
  struct message_t* mensagemSkel;
  struct pollfd connections[NFDESC];

  for (i = 0; i < NFDESC; i++)
    connections[i].fd = -1;

  connections[0].fd = listening_socket;
  connections[0].events = POLLIN;

  nfds = 1;

  while ((kfds = poll(connections, nfds, 10)) >= 0) // kfds == 0 significa timeout sem eventos

  if (kfds > 0){ // kfds é o número de descritores com evento ou erro

    if ((connections[0].revents & POLLIN) && (nfds < NFDESC))  // Pedido na listening socket ?
      if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Ligação feita ?
        connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
        nfds++;
    }
    for (i = 1; i < nfds; i++) // Todas as ligações

      if (connections[i].revents & POLLIN) { // Dados para ler ?

        mensagemSkel = network_receive(connections[i].fd);

        if(mensagemSkel == NULL){
            connections[i].fd = -1;
            break;
        }

        int fine = invoke(mensagemSkel);

        if(fine == -1){
          printf("Ocorreu um erro\n");
          continue;
        }

        fine = network_send(connections[i].fd, mensagemSkel);

        if(fine == -1){
          printf("Ocorreu um erro\n");
          continue;
        }

        if(mensagemSkel->mensagem.opcode == MESSAGE_T__OPCODE__OP_BAD){
          printf("Cliente %d fechou a ligação\n", connections[i].fd);
          close(connections[i].fd);
          connections[i].fd = -1;
        }

        if(connections[0].revents & POLLHUP){
          printf("Cliente %d fechou a ligação\n", connections[i].fd);
          close(connections[i].fd);
          connections[i].fd = -1;
        }
      }
  }
  return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
  tree_skel_destroy();
  close(sockfd);
  return 0;
}
