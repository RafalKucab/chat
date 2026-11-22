#include <_strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
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
#include "../common/dlink_list.h"

#define BUFFER_SIZE 1024

void usage(void) { fprintf(stderr, "usage: server [-p|--port PORT]\n"); }

// int add_to_pfds(int sock_client, struct pollfd *fds, uint8_t *fds_count) {
// for (uint8_t i = 1; i < MAX_FDS; i++) {
// if (fds[i].fd == -1) {
// fds[i].fd = sock_client;
// fds[i].events = POLLIN;
// fds[i].revents = 0;
// (*fds_count)++;
// return 0;
// }
// }
// return 1;
// }

// void del_from_pfds(int sock_client, struct pollfd *fds, uint8_t *fds_count) {
// for (uint8_t i = 1; i < MAX_FDS; i++) {
// if (fds[i].fd == sock_client) {
// fds[i].fd = -1;
// fds[i].events = 0;
// fds[i].revents = 0;
// (*fds_count)--;
// return;
// }
// }
// fprintf(stderr, "No client socket descriptor found\n");
// }

int test_connection(void *p1, void *p2) {
  connection_t *c1 = (connection_t *)p1;
  connection_t *c2 = (connection_t *)p2;
  return (c1->fd == c2->fd) ? 0 : 1;
}

connection_t *find_conn(dlink_list_t *conn_list, int fd) {
  connection_t pattern = {.fd = fd};
  return dlink_list_search(conn_list, &pattern, test_connection);
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

void broadcast(int sender_fd, const char *msg, size_t len,
               dlink_list_t *conn_list) {
  size_t conn_count = dlink_list_size(conn_list);
  for (uint8_t i = 0; i < conn_count; i++) {
    connection_t *conn = dlink_list_get_from(conn_list, i);
    if (conn->fd != sender_fd) {
      if (send_all(conn->fd, msg, len) < 0) {
        perror("Sending message failed");
      }
    }
  }
}

void handle_new_conn(int sock, dlink_list_t *conn_list) {
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

  // add connection to the list
  connection_t connection;
  bzero(&connection, sizeof(connection_t));
  connection.fd = sock_client;
  connection.port = client_addr.sin_port;
  if (inet_ntop(AF_INET, &client_addr.sin_addr, connection.ip,
                INET_ADDRSTRLEN) == NULL) {
    strcpy(remote_addr, "?");
  }
  dlink_list_insert_at_end(conn_list, &connection);

  // add client socket to the list of descriptors
  // if (add_to_pfds(sock_client, fds, fds_count) == 0) {
  // fprintf(stdout, "Accepted incoming connection from %s:%u\n", remote_addr,
  // ntohs(client_addr.sin_port));
  // } else {
  // fprintf(stderr, "Maximum number of connections reached\n");
  // close(sock_client);
  //}
}

void handle_client_data(dlink_list_t *conn_list, connection_t *conn) {
  char buffer[BUFFER_SIZE];
  // int sock_client = fds[idx].fd;

  ssize_t bytes_read = recv(conn->fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_read <= 0) {
    if (bytes_read == 0) {
      // connection closed
      fprintf(stdout, "Connection closed by peer\n");
    } else {
      // connection error
      perror("Receiving data failed");
    }

    // delete connection
    find_conn(conn_list, conn->fd);
    close(conn->fd);
    dlink_list_delete(conn_list, conn, test_connection);
    return;
  }

  // print read buffer and send it to other clients
  buffer[bytes_read] = '\0';
  fprintf(stdout, "%s", buffer);
  fflush(stdout);
  broadcast(conn->fd, buffer, (size_t)bytes_read, conn_list);
}

void process_conn(int server_sock, struct pollfd *fds,
                  dlink_list_t *conn_list) {
  // check server fd
  if (fds[0].revents & POLLIN) {
    handle_new_conn(server_sock, conn_list);
  }

  // check other fds
  for (uint8_t i = 1; i < dlink_list_size(conn_list) + 1; i++) {
    connection_t *conn = find_conn(conn_list, fds[i].fd);
    if (fds[i].fd >= 0 && (fds[i].revents & (POLLIN | POLLHUP | POLLERR))) {
      handle_client_data(conn_list, conn);
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

  dlink_list_t *conn_list = dlink_list_init(sizeof(connection_t));

  for (;;) {
    size_t conn_count = dlink_list_size(conn_list);
    struct pollfd fds_arr[conn_count + 1];
    fds_arr[0].fd = sock_serv;
    fds_arr[0].events = POLLIN;
    fds_arr[0].revents = 0;
    for (size_t i = 0; i < conn_count; i++) {
      connection_t *conn = dlink_list_get_from(conn_list, i);
      fds_arr[i + 1].events = POLLIN;
      fds_arr[i + 1].revents = 0;
      fds_arr[i + 1].fd = conn->fd;
    }
    int poll_count = poll(fds_arr, conn_count + 1, -1);
    if (poll_count < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("Poll failed");
      break;
    }

    process_conn(sock_serv, fds_arr, conn_list);
  }

  // close client sockets and server socket
  size_t conn_count = dlink_list_size(conn_list);
  for (size_t i = 0; i < conn_count; i++) {
    connection_t *conn = dlink_list_get_from(conn_list, i);
    close(conn->fd);
  }
  close(sock_serv);

  fprintf(stdout, "Shutting down the server\n");
  return EXIT_SUCCESS;
}
