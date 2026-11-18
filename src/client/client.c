#include <_strings.h>
#include <_time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/common.h"
#include "../common/list.h"
#include "frame.h"

typedef struct worker_params {
  char name[64];
  char addr[128];
  int sock;
  chat_list_t *chat_history;
} worker_params_t;

void usage(void) { fprintf(stderr, "usage: client [server_addr] [name]\n"); }

int open_conn(char *addr, unsigned int port) {
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));

  // create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary
  if (inet_pton(AF_INET, addr, &sock_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // initiate a connection
  int conn = connect(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
  if (conn < 0) {
    perror("Initiate connection failed");
    return 1;
  }

  return sock;
}

void *poll_worker(void *args) {
  worker_params_t worker_params = *(worker_params_t *)args;
  struct pollfd fds[2];
  char input[1024] = {0};
  char incomming[1024] = {0};
  char title[128];

  snprintf(title, sizeof(title), "Connected to %s", worker_params.addr);

  fds[0].fd = 0; // STDIN
  fds[0].events = POLLIN;
  fds[0].revents = 0;

  fds[1].fd = worker_params.sock;
  fds[1].events = POLLIN;
  fds[1].revents = 0;

  render(title, worker_params.chat_history, input, strlen(input));
  for (;;) {
    if (poll(fds, 2, -1) < 0) {
      fprintf(stdout, "poll quiting");
      break;
    }
    if (fds[0].revents & (POLLIN | POLLHUP | POLLERR)) {
      ssize_t n = read(0, input, sizeof(input) - 1);
      if (n <= 0) {
        break;
      }
      input[n] = '\0';
      msg_t msg;
      strcpy(msg.user, worker_params.name);
      strcpy(msg.message, input);
      send(worker_params.sock, &msg, sizeof(msg_t), 0);
      arr_push(worker_params.chat_history, &msg);
      render(title, worker_params.chat_history, input, strlen(input));
    }
    if (fds[1].revents & (POLLIN | POLLHUP | POLLERR)) {
      bzero(&incomming, 1024);
      ssize_t n = recv(fds[1].fd, incomming, sizeof(incomming) - 1, 0);
      if (n <= 0) { // server closed the connection or error
        break;
      }
      // incomming[n] = '\0';
      msg_t *msg = (msg_t *)&incomming;

      arr_push(worker_params.chat_history, msg);
      render(title, worker_params.chat_history, input, strlen(input));
    }
  }

  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage();
    return 1;
  }
  char *addr = argv[1];
  char *name = argv[2];

  chat_list_t *chat_history = arr_init(100);

  const int PORT = 8011;

  int sock = open_conn(addr, PORT);

  pthread_t poll_th;
  worker_params_t worker_params;
  bzero(&worker_params, sizeof(worker_params));
  strncpy(worker_params.addr, addr, strlen(addr));
  strncpy(worker_params.name, name, strlen(name));
  worker_params.sock = sock;
  worker_params.chat_history = chat_history;

  if (pthread_create(&poll_th, NULL, poll_worker, &worker_params) != 0) {
    perror("Thread creation failed");
    close(sock);
    return 1;
  }

  pthread_join(poll_th, NULL);

  return 0;
}
