#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "include/sdmessage.pb-c.h"
#include "message-private.h"
#include "inet.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

int write_all(int sockfd, char* buf, int size){
  signal(SIGPIPE, SIG_IGN);
  int bufsize = size;
  while(size>0){
    int res = write(sockfd, buf, size);
    if(res<0){
      if(errno == EINTR) continue;
      perror("write failed:");
      return res;
    }
    buf += res;
    size -= res;
  }
  return bufsize;
}

int read_all(int sockfd, char* buf, int size){
  int bufsize = size;
  while(size>0){
    int res = read(sockfd, buf, size);
    if(res<0){
      if(errno == EINTR) continue;
      perror("read failed:");
      return res;
    }
    buf += res;
    size -= res;
  }
  return bufsize;
}
