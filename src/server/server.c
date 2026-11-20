#include <_strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/common.h"

#define MAX_CLIENTS 3
#define MAX_FDS (MAX_CLIENTS + 1) // slot 0 is the listening socket
#define BUFFER_SIZE 1024

void usage(void) { fprintf(stderr, "usage: server [-p|--port PORT]\n"); }

int add_to_pfds(int sock_client, struct pollfd *fds, uint8_t *fds_count) {
  for (uint8_t i = 1; i < MAX_FDS; i++) {
    if (fds[i].fd == -1) {
      fds[i].fd = sock_client;
      fds[i].events = POLLIN;
      fds[i].revents = 0;
      (*fds_count)++;
      return 0;
    }
  }
  return 1;
}

void del_from_pfds(int sock_client, struct pollfd *fds, uint8_t *fds_count) {
  for (uint8_t i = 1; i < MAX_FDS; i++) {
    if (fds[i].fd == sock_client) {
      fds[i].fd = -1;
      fds[i].events = 0;
      fds[i].revents = 0;
      (*fds_count)--;
      return;
    }
  }
  fprintf(stderr, "No client socket descriptor found\n");
}

void handle_new_conn(int sock, struct pollfd *fds, uint8_t *fds_count) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char remote_addr[INET_ADDRSTRLEN];

  // accept new client connection
  int sock_client =
      accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
  if (sock_client < 0) {
    perror("Accept connection failed");
    return;
  }
  connection_t connection;
  bzero(&connection, sizeof(connection_t));
  connection.fd = sock_client;
  connection.port = client_addr.sin_port;
  if (inet_ntop(AF_INET, &client_addr.sin_addr, connection.ip,
                INET_ADDRSTRLEN) == NULL) {
    strcpy(remote_addr, "?");
  }

  // add client socket to the list of descriptors
  if (add_to_pfds(sock_client, fds, fds_count) == 0) {
    fprintf(stdout, "Accepted incoming connection from %s:%u\n", remote_addr,
            ntohs(client_addr.sin_port));
  } else {
    fprintf(stderr, "Maximum number of connections reached\n");
    close(sock_client);
  }
}

// send the whole buffer, retrying on partial sends
int send_all(int sock, const char *msg, size_t len) {
  size_t total = 0;
  while (total < len) {
    ssize_t n = send(sock, msg + total, len - total, 0);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    total += (size_t)n;
  }
  return 0;
}

void broadcast(struct pollfd *fds, const char *msg, size_t len, int sock_from) {
  for (uint8_t i = 1; i < MAX_FDS; i++) {
    if (fds[i].fd >= 0 && fds[i].fd != sock_from) {
      if (send_all(fds[i].fd, msg, len) < 0) {
        perror("Sending message failed");
      }
    }
  }
}

void handle_client_data(struct pollfd *fds, uint8_t idx, uint8_t *fds_count) {
  char buffer[BUFFER_SIZE];
  int sock_client = fds[idx].fd;

  ssize_t bytes_read = recv(sock_client, buffer, sizeof(buffer) - 1, 0);
  if (bytes_read <= 0) {
    if (bytes_read == 0) {
      // connection closed
      fprintf(stdout, "Connection closed by peer\n");
    } else {
      // connection error
      perror("Receiving data failed");
    }

    close(sock_client);
    del_from_pfds(sock_client, fds, fds_count);
    return;
  }

  buffer[bytes_read] = '\0';
  fprintf(stdout, "%s", buffer);
  fflush(stdout);
  broadcast(fds, buffer, (size_t)bytes_read, sock_client);
}

void process_conn(int server_sock, struct pollfd *fds, uint8_t *fds_count) {
  // check server fd
  if (fds[0].revents & POLLIN) {
    handle_new_conn(server_sock, fds, fds_count);
  }

  // check other fds
  for (uint8_t i = 1; i < MAX_FDS; i++) {
    if (fds[i].fd >= 0 && (fds[i].revents & (POLLIN | POLLHUP | POLLERR))) {
      handle_client_data(fds, i, fds_count);
    }
  }
}

int create_sock(uint16_t port) {
  // create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return -1;
  }

  int yes = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    perror("Setting SO_REUSEADDR failed");
    close(sock);
    return -1;
  }

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind socket
  if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Bind socket failed");
    close(sock);
    return -1;
  }

  // listen incoming connections
  if (listen(sock, SOMAXCONN) < 0) {
    perror("Listen socket failed");
    close(sock);
    return -1;
  }

  return sock;
}

int parse_port(const char *str, uint16_t *port) {
  char *end = NULL;
  errno = 0;
  long val = strtol(str, &end, 10);
  if (errno != 0 || end == str || *end != '\0' || val < 1 || val > 65535) {
    return -1;
  }
  *port = (uint16_t)val;
  return 0;
}

int main(int argc, char **argv) {
  uint16_t port = 8012;

  if (argc == 3 &&
      (strcmp("-p", argv[1]) == 0 || strcmp("--port", argv[1]) == 0)) {
    if (parse_port(argv[2], &port) < 0) {
      fprintf(stderr, "Invalid port: %s\n", argv[2]);
      usage();
      return EXIT_FAILURE;
    }
  } else if (argc != 1) {
    usage();
    return EXIT_FAILURE;
  }

  printf("Starting server on port %u...\n", port);

  int sock_serv = create_sock(port);
  if (sock_serv < 0) {
    return EXIT_FAILURE;
  }

  struct pollfd fds_arr[MAX_FDS];
  for (uint8_t i = 0; i < MAX_FDS; i++) {
    fds_arr[i].fd = -1;
    fds_arr[i].events = 0;
    fds_arr[i].revents = 0;
  }

  fds_arr[0].fd = sock_serv;
  fds_arr[0].events = POLLIN;
  fds_arr[0].revents = 0;
  uint8_t fds_count = 1; // server fd

  for (;;) {
    int poll_count = poll(fds_arr, MAX_FDS, -1);
    if (poll_count < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("Poll failed");
      break;
    }

    process_conn(sock_serv, fds_arr, &fds_count);
  }

  // close client sockets and server socket
  for (uint8_t i = 1; i < MAX_FDS; i++) {
    if (fds_arr[i].fd >= 0) {
      close(fds_arr[i].fd);
    }
  }
  close(sock_serv);

  fprintf(stdout, "Shutting down the server\n");
  return EXIT_SUCCESS;
}
