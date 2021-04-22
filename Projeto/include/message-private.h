#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "sdmessage.pb-c.h"


struct message_t{
  MessageT mensagem;
};

int write_all(int sockfd, char* buf, int size);

int read_all(int sockfd, char* buf, int msg_size);
#endif
