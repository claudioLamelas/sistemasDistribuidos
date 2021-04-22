#define THREADED
#include <pthread.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "message-private.h"
#include "inet.h"
#include "tree_skel-private.h"
#include "tree_skel.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "network_client.h"
#include "network_server.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

int last_assigned, op_count;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
struct task_t* queue_head;

struct rtree_t* rtreeglobal;
struct tree_t* treeglobal;
int buffer_len = 100;
char* buffer;

#define ZDATALEN 1024

static char* connection;
typedef struct String_vector zoo_string;
static zhandle_t* zh;
static int is_connected;
static const char* root_path = "/kvstore";
static char *watcher_ctx = "ZooKeeper Data Watcher";

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	int new_path_len = 1024;
	char* new_path = malloc(new_path_len);
	const char* primary = "primary";
	const char* backup = "backup";
	int pExists = 0;
	int bExists = 0;
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;
	if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT) {
			sleep(3);
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
			if(!pExists && bExists){
				zoo_create(zh, "/kvstore/primary", connection, ZDATALEN, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, new_path, new_path_len);
				zoo_delete(zh, "/kvstore/backup", -1);
				bExists = 0;
				pExists = 1;
			}
			else if(bExists){
				buffer = malloc(buffer_len);
				zoo_get(zh,"/kvstore/backup", 0, buffer, &buffer_len, NULL);

				rtreeglobal = rtree_connect(buffer);

			}else{
				printf("Ligacao ao backup fechada\n");
				network_close(rtreeglobal);
			}
			}
		 }
		 free(children_list);
	 }

/* Connect to ZooKeeper server */
void connect_zookeeper(char* porta, char* hostname){
	char* address_porta = strdup(hostname);
	char* s = ":";
	rtreeglobal = malloc(sizeof(struct rtree_t));
	zh = zookeeper_init(hostname, connection_watcher,	2000, 0, 0, 0);
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}
	rtreeglobal->port = porta;
	rtreeglobal->hostname = strtok(address_porta,s);
	connection = strcat(rtreeglobal->hostname, ":");
	connection = strcat(connection, porta);
}

/* Função do thread secundário que vai processar pedidos de escrita.*/
void *process_task(void* params){
	int *result = 0;

	while (1){

		pthread_mutex_lock(&queue_lock);
		/* Esperar por dados > 0 */
		while (queue_head == NULL){
			pthread_cond_wait(&queue_not_empty, &queue_lock);
    }

    pthread_mutex_lock(&tree_lock);
		if (queue_head->op == 0){
      tree_del(treeglobal, queue_head->key);
      op_count += 1;
    } else {
      tree_put(treeglobal, queue_head->key, queue_head->dados);
      op_count += 1;
    }

		if(rtreeglobal->sockfd != -1){
			queue_head->msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_PUT;
			network_send(rtreeglobal->sockfd, queue_head->msg);
		}

		queue_head = queue_head->next;

    pthread_mutex_unlock(&tree_lock);
		pthread_mutex_unlock(&queue_lock);

	}
}

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */

int tree_skel_init(){

  last_assigned = 0;
  op_count = 0;
  queue_head = malloc(sizeof(struct task_t));
  queue_head = NULL;
  pthread_t nova;

	sleep(3); /* Sleep a little for connection to complete */

	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

		if(is_connected){
			int new_path_len = 1024;
			char* new_path = malloc(new_path_len);
			if (ZNONODE == zoo_exists(zh, "/kvstore", 0, NULL)){
				printf("O kvstore ainda nao existe\n");
				zoo_create(zh, "/kvstore", NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, new_path, new_path_len);
				zoo_create(zh, "/kvstore/primary", connection, ZDATALEN, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, new_path, new_path_len);
				printf("O kvstore foi criado com sucesso\n");
			}else if(ZNONODE != zoo_exists(zh, "/kvstore/primary", 0, NULL) && ZNONODE != zoo_exists(zh, "/kvstore/backup", 0, NULL)){
				printf("Ambos os servidores estão up\n");
				return -1;
			}else if(ZNONODE == zoo_exists(zh, "/kvstore/primary", 0, NULL)){
				printf("O primary ainda nao existe\n");
				if(ZOK != zoo_create(zh, "/kvstore/primary", connection, ZDATALEN, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, new_path, new_path_len)){
					printf("Erro ao criar o kvstore/primary\n");
				};
				printf("O primary foi criado com sucesso\n");
			}else{
				printf("O backup ainda nao existe\n");
				if(ZOK != zoo_create(zh, "/kvstore/backup", connection, ZDATALEN, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, new_path, new_path_len)){
					printf("Erro ao criar o kvstore/backup\n");
				};
				rtreeglobal->sockfd = -1;
				printf("O backup foi criado com sucesso\n");
			}
			free(new_path);

			if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", root_path);
			}
		}else{
			printf("Nao foi possivel conectar-se\n");
		}

  if((treeglobal = tree_create()) == NULL){
    return -1;
  }

  if (pthread_create(&nova, NULL, &process_task, NULL) != 0){
		perror("\nThread não criada.\n");
		exit(EXIT_FAILURE);
	}

	free(children_list);
  return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy(){
  tree_destroy(treeglobal);
}

int verify(int op_n){
  pthread_mutex_lock(&queue_lock);
  if(queue_head == NULL){
    pthread_mutex_unlock(&queue_lock);
    return op_n <= last_assigned;
  }
  while(queue_head != NULL){
    struct task_t* task= queue_head;
    if(task->op_n == op_n){
      pthread_mutex_unlock(&queue_lock);
      return 0;
    }
    queue_head = task->next;
  }
  pthread_mutex_unlock(&queue_lock);
  return 1;
}

void queue_add_task(struct task_t* task) {
  pthread_mutex_lock(&queue_lock);
  if(queue_head==NULL) { /* Adiciona na cabeça da fila */
    queue_head= task;
    task->next=NULL;
  }
  else{ /* Adiciona no fim da fila */
    struct task_t* tptr = queue_head;
    while(tptr->next!= NULL) tptr = tptr->next;
    tptr->next=task;
    task->next=NULL;
  }
  pthread_cond_signal(&queue_not_empty);/* Avisa um bloqueado nessa condição */
  pthread_mutex_unlock(&queue_lock);
}

struct task_t* queue_get_task(){
  pthread_mutex_lock(&queue_lock);
  while(queue_head==NULL)
    pthread_cond_wait(&queue_not_empty, &queue_lock); /* Espera haver algo */
  struct task_t* task = queue_head;
  queue_head = task->next;
  pthread_mutex_unlock(&queue_lock);
  return task;
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao inicializada)
*/
int invoke(struct message_t *msg){
    struct task_t* tarefa;
    struct data_t* data;
    switch (msg->mensagem.opcode) {
      case MESSAGE_T__OPCODE__OP_SIZE:
        msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->mensagem.tree_size = tree_size(treeglobal);
        op_count += 1;
        return 0;
      case MESSAGE_T__OPCODE__OP_DEL:
        if(treeglobal == NULL){
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_ERROR;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
          return -1;
        }

        last_assigned += 1;
        tarefa = malloc(sizeof(struct task_t));
        tarefa->op_n = last_assigned;
        tarefa->op = 0;
        tarefa->key = msg->mensagem.key;
        tarefa->data = NULL;
        tarefa->next = NULL;
				tarefa->msg = msg;

        queue_add_task(tarefa);

        msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
        msg->mensagem.op_n = last_assigned;
        return 0;
      case MESSAGE_T__OPCODE__OP_GET:
        if(treeglobal == NULL){
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_ERROR;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
          return -1;
        }
        data = tree_get(treeglobal, msg->mensagem.key);
        op_count += 1;
        if(data == NULL){
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
          msg->mensagem.data_size = 0;
          msg->mensagem.data = NULL;
          return 0;
        }else{
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
          msg->mensagem.data_size = data->datasize;
          msg->mensagem.data = data->data;
          return 0;
        }
      case MESSAGE_T__OPCODE__OP_PUT:
        if(treeglobal == NULL){
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_ERROR;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
          return -1;
        }
        data = data_create2(msg->mensagem.data_size, msg->mensagem.data);

        last_assigned += 1;
        tarefa = malloc(sizeof(struct task_t));
        tarefa->op_n = last_assigned;
        tarefa->op = 1;
        tarefa->key = msg->mensagem.key;
        tarefa->data = msg->mensagem.data;
        tarefa->dados = data;
        tarefa->next = NULL;
				tarefa->msg = msg;

        queue_add_task(tarefa);

        msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
        msg->mensagem.op_n = last_assigned;

        return 0;
      case MESSAGE_T__OPCODE__OP_GETKEYS:
        if(treeglobal == NULL){
          return -1;
        }else{
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
          msg->mensagem.keys = tree_get_keys(treeglobal);
          op_count += 1;
          return 0;
        }
      case MESSAGE_T__OPCODE__OP_HEIGHT:
        msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->mensagem.tree_height = tree_height(treeglobal);
        op_count += 1;
        return 0;
      case MESSAGE_T__OPCODE__OP_VERIFY:
        if(treeglobal == NULL){
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_ERROR;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
          return -1;
        }else{
          msg->mensagem.opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
          msg->mensagem.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
          msg->mensagem.result = verify(msg->mensagem.op_n);
          return 0;
        }
      case MESSAGE_T__OPCODE__OP_BAD:
        return 0;
    }
    return 0;
  }
