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
#include "frame.h"

#define MAX_LINES 80

char *lines[MAX_LINES] = {0};

typedef struct worker_params {
  char name[64];
  char addr[128];
  int sock;
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

// append a copy of new_line (len bytes, not NUL-terminated) to the history;
// when the history is full the oldest line is dropped
void append_line(char **lines, size_t lines_num, const char *new_line,
                 size_t len, uint8_t *line_count) {
  // strip trailing newline(s)
  while (len > 0 && (new_line[len - 1] == '\n' || new_line[len - 1] == '\r')) {
    len--;
  }

  char *copy = malloc(len + 1);
  if (copy == NULL) {
    return;
  }
  memcpy(copy, new_line, len);
  copy[len] = '\0';

  if (*line_count == lines_num) { // full: drop oldest, shift the rest up
    free(lines[0]);
    memmove(&lines[0], &lines[1], (lines_num - 1) * sizeof(char *));
    (*line_count)--;
  }
  lines[(*line_count)++] = copy;
}

void *poll_worker(void *args) {
  worker_params_t worker_params = *(worker_params_t *)args;
  struct pollfd fds[2];
  char input[1024] = {0};
  char incomming[1024] = {0};
  uint8_t line_count = 0;
  char title[128];

  snprintf(title, sizeof(title), "Connected to %s", worker_params.addr);

  fds[0].fd = 0; // STDIN
  fds[0].events = POLLIN;
  fds[0].revents = 0;

  fds[1].fd = worker_params.sock;
  fds[1].events = POLLIN;
  fds[1].revents = 0;

  render(title, worker_params.name, (const char **)lines, line_count, input,
         strlen(input) - 2);
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
      strncpy(msg.user, worker_params.name, strlen(worker_params.name));
      strncpy(msg.message, input, strlen(input));
      send(worker_params.sock, &msg, sizeof(msg_t), 0);
      append_line(lines, MAX_LINES, input, (size_t)n, &line_count);
      render(title, worker_params.name, (const char **)lines, line_count, input,
             strlen(input) - 2);
    }
    if (fds[1].revents & (POLLIN | POLLHUP | POLLERR)) {
      ssize_t n = recv(fds[1].fd, incomming, sizeof(incomming) - 1, 0);
      if (n <= 0) { // server closed the connection or error
        break;
      }
      incomming[n] = '\0';
      msg_t *msg = (msg_t *)&incomming;

      append_line(lines, MAX_LINES, msg->message, (size_t)n, &line_count);
      render(title, msg->user, (const char **)lines, line_count, input,
             strlen(input) - 2);
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

  const int PORT = 8011;

  int sock = open_conn(addr, PORT);

  pthread_t poll_th;
  worker_params_t worker_params;
  strncpy(worker_params.addr, addr, strlen(addr));
  strncpy(worker_params.name, name, strlen(name));
  worker_params.sock = sock;

  if (pthread_create(&poll_th, NULL, poll_worker, &worker_params) != 0) {
    perror("Thread creation failed");
    close(sock);
    return 1;
  }

  pthread_join(poll_th, NULL);

  return 0;
}
