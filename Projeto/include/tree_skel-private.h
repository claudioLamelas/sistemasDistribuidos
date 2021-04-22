#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "message-private.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

struct task_t{
  int op_n; //o número da operação
  int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
  char* key; //a chave a remover ou adicionar
  char* data; //os dados a adicionar em caso de put, ou NULL em caso de delete
  struct data_t* dados;
  //adicionar campo(s) necessário(s)para implementar fila do tipo produtor/consumido
  struct task_t* next;
  struct message_t* msg;
};

void queue_init();

void queue_add_task(struct task_t* task);

struct task_t* queue_get_task();

void connect_zookeeper(char* porta, char* hostname);

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

#endif
