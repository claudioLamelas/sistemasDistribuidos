#define THREADED
#include <errno.h>

#include "client_stub.h"
#include "inet.h"
#include "client_stub-private.h"
#include "include/sdmessage.pb-c.h"
#include "network_client.h"
#include "message-private.h"
#include <zookeeper/zookeeper.h>

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

struct rtree_t;
static zhandle_t* zh;
struct rtree_t* rtreeglobal;
static char* connection;
static int is_connected;

const char* primary = "primary";
const char* backup = "backup";
int pExists = 0;
int bExists = 0;

typedef struct String_vector zoo_string;
static const char* root_path = "/kvstore";
static char *watcher_ctx = "ZooKeeper Data Watcher";
int tamanho = 100;
char* primary_hostport;
char* backup_hostport;
struct rtree_t* rtree_primary = NULL;
struct rtree_t* rtree_backup = NULL;
int primary_connected, backup_connected;

void update_client(struct rtree_t* rtree_primary_client, struct rtree_t* rtree_backup_client, int* podesEscrever_client){
  rtree_primary_client = rtree_primary;
  rtree_backup_client = rtree_backup;
	int podesEscrever = primary_connected && backup_connected;
	*podesEscrever_client = podesEscrever;
}

void connection_watcher_client(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	pExists = 0;
	bExists = 0;
	if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT) {
	 	   /* Get the updated children and reset the watch */
			 if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", root_path);
 			}
			for (int i = 0; i < children_list->count; i++)  {
				if(strcmp(children_list->data[i], primary) == 0){
					pExists = 1;
				}
				if(strcmp(children_list->data[i], backup) == 0){
					bExists = 1;
				}
			}
			if(pExists && bExists){
				primary_hostport = malloc(tamanho);
				backup_hostport = malloc(tamanho);
				zoo_get(zh, "/kvstore/primary", 0, primary_hostport, &tamanho, NULL);
        zoo_get(zh, "/kvstore/backup", 0, backup_hostport, &tamanho, NULL);
        if (!primary_connected){
          rtree_primary = rtree_connect(primary_hostport);
          primary_connected = 1;
        }
        if (!backup_connected){
          rtree_backup = rtree_connect(backup_hostport);
          backup_connected = 1;
        }
			}
			if(!bExists){
				if(backup_connected){
					rtree_disconnect(rtree_backup);
				}
				backup_connected = 0;
			}
      if(!pExists){
				if(primary_connected){
					rtree_disconnect(rtree_primary);
				}
        primary_connected = 0;
			}
			}
		 }
		 free(children_list);
	 }

struct rtree_t *getrtree_primary(){
  return rtree_primary;
}

struct rtree_t *getrtree_backup(){
  return rtree_backup;
}

int getBool(){
  return primary_connected && backup_connected;
}
/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){

  struct rtree_t* rtree = malloc(sizeof(struct rtree_t));
  char* address_porta = strdup(address_port);
  char* s = ":";
  struct sockaddr_in server;

  rtree->hostname = strtok(address_porta,s);
  rtree->port = strtok(NULL,s);

  // Cria socket TCP
  if ((rtree->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      printf("Erro ao converter IP\n");
      perror("Erro ao criar socket TCP");
      return NULL;
  }

  // Preenche estrutura server com endereço do servidor para estabelecer
  // conexão

  server.sin_family = AF_INET; // família de endereços
  server.sin_port = htons(atoi(rtree->port)); // Porta TCP
  if (inet_pton(AF_INET, rtree->hostname, &server.sin_addr) < 1) { // Endereço IP
      printf("Erro ao converter IP\n");
      close(rtree->sockfd);
      return NULL;
  }

  rtree->serverIn = server;

  int fine;
  if((fine = network_connect(rtree)) == -1){
    return NULL;
  }

  free(address_porta);
  return rtree; // Termina
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree){
  int fine = network_close(rtree);
  free(rtree);
  return fine;
}

void connect_zookeeper_client(char* hostport){
	char* address_porta = strdup(hostport);
	char* s = ":";
  char* hostname = strtok(address_porta,s);
  char* porta = strtok(NULL, s);

	rtreeglobal = malloc(sizeof(struct rtree_t));
	zh = zookeeper_init(hostport, connection_watcher_client,	2000, 0, 0, 0);
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}
	rtreeglobal->port = porta;
	rtreeglobal->hostname = hostname;
	connection = hostport;
}

void watch_children(){
  zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	pExists = 0;
	bExists = 0;

  if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
    fprintf(stderr, "Error setting watch at %s!\n", root_path);
  }
	for (int i = 0; i < children_list->count; i++)  {
		if(strcmp(children_list->data[i], primary) == 0){
			pExists = 1;
		}
		if(strcmp(children_list->data[i], backup) == 0){
			bExists = 1;
		}
	}
	if(pExists && bExists){
		primary_hostport = malloc(tamanho);
		backup_hostport = malloc(tamanho);
		zoo_get(zh, "/kvstore/primary", 0, primary_hostport, &tamanho, NULL);
		zoo_get(zh, "/kvstore/backup", 0, backup_hostport, &tamanho, NULL);
		if (!primary_connected){
			rtree_primary = rtree_connect(primary_hostport);
			primary_connected = 1;
		}
		if (!backup_connected){
			rtree_backup = rtree_connect(backup_hostport);
			backup_connected = 1;
		}
	}
	if(!bExists){
		if(backup_connected){
			rtree_disconnect(rtree_backup);
		}
		backup_connected = 0;
	}
	if(!pExists){
		if(primary_connected){
			rtree_disconnect(rtree_primary);
		}
		primary_connected = 0;
	}
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_PUT;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
  mensagem.data_size = entry->value->datasize;
  mensagem.data = entry->value->data;
  mensagem.key = entry->key;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return -1;
  }
  free(msgSend);
  message_t__free_unpacked(&msgReceive->mensagem, NULL);
  return msgReceive->mensagem.op_n;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_GET;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_KEY;
  mensagem.key = key;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return NULL;
  }

  struct data_t* dados;

  dados = data_create2(msgReceive->mensagem.data_size,msgReceive->mensagem.data);
  free(msgSend);
  free(msgReceive);
  return dados;
}

/* Função para remover um elemento da árvore. Vai libertar
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_DEL;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_KEY;
  mensagem.key = key;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return -1;
  }
  if(msgReceive->mensagem.opcode == MESSAGE_T__OPCODE__OP_ERROR){
    return -1;
  }

  return msgReceive->mensagem.op_n;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_SIZE;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return -1;
  }

  int size = msgReceive->mensagem.tree_size;
  message_t__free_unpacked(&msgReceive->mensagem, NULL);
  free(msgSend);
  return size;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;
  int height;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return -1;
  }

  height = msgReceive->mensagem.tree_height;
  message_t__free_unpacked(&msgReceive->mensagem, NULL);
  free(msgSend);

  return height;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return NULL;
  }

  int size = rtree_size(rtree);

  char** keys = malloc(sizeof(char*)*size);

  if(msgReceive->mensagem.keys != NULL){

  for (int i = 0; i < size; i++){
    if (msgReceive->mensagem.keys[i] != NULL){
      char* chave = msgReceive->mensagem.keys[i];
      keys[i] = strdup(chave);
      keys[i+1] = NULL;
    }
  }
}

  return keys;
}

int rtree_verify(struct rtree_t *rtree,int op_n){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
  mensagem.op_n = op_n;

  msgSend->mensagem = mensagem;

  if((msgReceive = network_send_receive(rtree, msgSend)) == NULL){
    return -1;
  }
  if(msgReceive->mensagem.opcode == MESSAGE_T__OPCODE__OP_ERROR){
    return -1;
  }

  return msgReceive->mensagem.result;
}

/* Liberta a memória alocada por rtree_get_keys().
 */
void rtree_free_keys(char **keys){
  int i;
	for(i = 0; keys[i] != NULL; i++){
		free(keys[i]);
	}
	free(keys);
}

void rtree_quit(struct rtree_t *rtree){
  struct message_t* msgSend;
  MessageT mensagem;
  struct message_t* msgReceive;

  msgSend = malloc(sizeof(struct message_t));

  message_t__init(&mensagem);

  mensagem.opcode = MESSAGE_T__OPCODE__OP_BAD;
  mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;

  msgSend->mensagem = mensagem;

  msgReceive = network_send_receive(rtree, msgSend);
  free(msgReceive);
  free(msgSend);
}
