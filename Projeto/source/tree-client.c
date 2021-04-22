#include "client_stub.h"
#include "client_stub-private.h"
#include "include/sdmessage.pb-c.h"
#include "inet.h"
#include <errno.h>
#include <signal.h>

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

struct rtree_t* rtree_primary;
struct rtree_t* rtree_backup;
int podesEscrever;




int main(int argc, char **argv){

  signal(SIGPIPE, SIG_IGN);

  if(argc != 2){
    printf("Use o formato: tree-client <server>:<port> (Zookeeper)\n");
    return -1;
  }

  connect_zookeeper_client(argv[1]);
  watch_children();

  do {
    char* tokens[MAX_MSG];
    int count, nbytes;
    char msg[MAX_MSG];
    printf("Por favor insira um comando:");
    fgets(msg,MAX_MSG,stdin);
    update_client(rtree_primary, rtree_backup, &podesEscrever);

    if(podesEscrever){
      int n = 0;

      for (char * p = strtok(msg, " "); p; p = strtok(NULL, " ")){
        if(n > 2){
          char* cena = strdup(p);
          tokens[2] = strcat(tokens[2], " ");
          tokens[2] = strcat(tokens[2], cena);
          free(cena);
        }
        else{
          tokens[n] = p;
          n += 1;
        }
      }

    if (strcmp(tokens[0], "put") == 0 && n >= 3){
      struct data_t* data;
      struct entry_t* entrada;
      data = data_create2(strlen(tokens[2]) - 1, tokens[2]);
      entrada = entry_create(tokens[1], data);
      int fine = rtree_put(rtree_primary, entrada);
      if (fine == -1){
        printf("Put mal sucedido\n");
      }else{
        printf("Put bem recebido. Operação Nº%d\n", fine);
      }
    }
    else if (strcmp(tokens[0], "get") == 0){
      struct data_t* data;
      char* key;
      key = strtok(tokens[1],"\n");
      data = rtree_get(rtree_backup, key);
      char* dados;
      if(data == NULL){
        printf("Nao foi possivel obter os dados\n");
      }
      else{
        dados = strndup(data->data, data->datasize);
        printf("Datasize: %d\n", data->datasize);
        printf("Data: %s\n", dados);
        free(dados);
        data_destroy(data);
      }
    }
    else if (strcmp(tokens[0], "del") == 0){
      char* key = strtok(tokens[1], "\n");
      int fine = rtree_del(rtree_primary, key);
      if (fine == -1){
        printf("Del mal sucedido\n");
      }else{
        printf("Del bem recebido. Operação Nº%d\n", fine);
      }
    }
    else if (strcmp(tokens[0], "size\n") == 0){
      int size = rtree_size(rtree_backup);
      printf("A árvore tem tamanho: %d\n", size);
    }
    else if (strcmp(tokens[0], "height\n") == 0){
      int height = rtree_height(rtree_backup);
      printf("A árvore tem altura: %d\n", height);
    }
    else if (strcmp(tokens[0], "getkeys\n") == 0){
      char** keys;
      keys = rtree_get_keys(rtree_backup);
      printf("Chaves:\n");
    }
    else if (strcmp(tokens[0], "verify") == 0){
      int op_n = atoi(strtok(tokens[1],"\n"));
      int done = rtree_verify(rtree_backup, op_n);
      printf("Feito: %d\n", done);
    }
    else if (strcmp(tokens[0], "quit\n") == 0){
      rtree_quit(rtree_primary);
      rtree_quit(rtree_backup);
      break;
    }else{
      printf("Comando inválido\n");
    }
  }else{
    printf("Um dos servidores desconectou-se.\n");
    printf("Tente novamente mais tarde.\n");
  }
  } while(1);
  rtree_disconnect(rtree_primary);
  rtree_disconnect(rtree_backup);
}
