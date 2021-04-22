#include "message-private.h"
#include "include/sdmessage.pb-c.h"
#include "network_server.h"
#include "tree_skel.h"
#include "tree_skel-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "inet.h"
#include <errno.h>
#include <signal.h>

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

int not_closed = 1;

int main(int argc, char **argv){

  if(argc != 3){
    printf("Use o formato: tree-server <port> <hostport>(ZooKeeper)\n");
    return -1;
  }

  void handler(int num){
    not_closed = 0;
  }

  int socket;

  signal(SIGINT, handler);

  while(not_closed){

  socket = network_server_init(atoi(argv[1]));

  if(socket == -1){
    break;
  }

  connect_zookeeper(argv[1], argv[2]);

  int fine = tree_skel_init();

  if(-fine){
    printf("Servidor a fechar.\n");
    network_server_close();
    return 1;
  }

  int result = network_main_loop(socket);
  }

  printf("Servidor a fechar.\n");
  network_server_close();
}
