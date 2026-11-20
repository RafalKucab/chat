#ifndef COMMON_H
#define COMMON_H

#include <netinet/in.h>

#define MAX_USERNAME_LEN 64
#define MAX_MESSAGE_LEN 256

typedef struct Connection {
  int fd;
  char ip[INET_ADDRSTRLEN];
  uint16_t port;
} connection_t;

typedef struct Message {
  char user[MAX_USERNAME_LEN];
  char message[MAX_MESSAGE_LEN];
} msg_t;

#endif
