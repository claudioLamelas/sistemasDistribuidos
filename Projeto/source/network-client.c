#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "include/sdmessage.pb-c.h"
#include "client_stub-private.h"
#include "message-private.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree){
  // Estabelece conexão com o servidor definido na estrutura server
  if (connect(rtree->sockfd,(struct sockaddr *)&rtree->serverIn, sizeof(rtree->serverIn)) < 0) {
      perror("Erro ao conectar-se ao servidor");
      rtree_disconnect(rtree);
      close(rtree->sockfd);
      return -1;
  }

  return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){

  int sockfd = rtree->sockfd;
  int tamanho = message_t__get_packed_size(&(msg->mensagem));
  int tamanho2;
  char* buf;
  int nbytes;
  MessageT* msgR;
  struct message_t* msgReceived;
  msgReceived = malloc(sizeof(struct message_t));

  tamanho2 = htonl(tamanho);

  buf = malloc(tamanho);

  if (buf == NULL) {
        fprintf(stdout, "malloc error\n");
        return NULL;
    }

  message_t__pack(&msg->mensagem, buf);

  if((nbytes = write_all(sockfd, (char*) &tamanho2, sizeof(int)) != sizeof(int))){
        perror("Erro ao enviar dados ao servidor");
        close(sockfd);
        return NULL;
    }

  if((nbytes = write_all(sockfd, buf, tamanho) != tamanho)){
      perror("Erro ao enviar dados ao servidor");
      close(sockfd);
      return NULL;
    }

  if((nbytes = read_all(sockfd,(char*) &tamanho,sizeof(int))) != sizeof(int)){
      perror("Erro ao receber dados do servidor");
      close(sockfd);
      return NULL;
  }

  char* bufResposta;
  free(buf);
  tamanho2 = ntohl(tamanho);
  bufResposta = malloc(tamanho2);

  if((nbytes = read_all(sockfd,bufResposta,tamanho2)) != tamanho2){
      perror("Erro ao receber dados do servidor");
      close(sockfd);
      return NULL;
  }

  msgR = message_t__unpack(NULL, tamanho2, bufResposta);
    if (msgR == NULL) {
        fprintf(stdout, "error unpacking message\n");
        return NULL;
    }

  msgReceived->mensagem = *msgR;
  free(bufResposta);

  return msgReceived;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
  int result = close(rtree->sockfd); // Fecha o socket
  return result;
}
