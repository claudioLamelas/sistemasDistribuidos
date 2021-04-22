#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "entry.h"
#include "inet.h"
#include "sdmessage.pb-c.h"
#include <zookeeper/zookeeper.h>

/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t{
  char* hostname;
  char* port;
  struct sockaddr_in serverIn;
  int sockfd;  //Porta;
};

void rtree_quit(struct rtree_t *rtree);
void connect_zookeeper_client(char* hostport);
void connection_watcher_client(zhandle_t *zzh, int type, int state, const char *path, void* context);
void update_client(struct rtree_t* rtree_primary, struct rtree_t* rtree_backup, int* podesEscrever);
void watch_children();

#endif
