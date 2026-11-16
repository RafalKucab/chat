#include <_strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_types/_u_int8_t.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const uint8_t MAX_CLIENT = 4;

void usage() { fprintf(stdout, "usage: server [-p|--port PORT\n"); }

int add_to_pfds(int sock_client, struct pollfd *fds, uint8_t *fds_count) {
  for (uint8_t i = 1; i < MAX_CLIENT; i++) {
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
void del_from_pfds(int sock_client, struct pollfd *fds) {
  for (uint8_t i = 1; i < MAX_CLIENT; i++) {
    if (fds[i].fd == sock_client) {
      fds[i].fd = -1;
      fds[i].events = 0;
      fds[i].revents = 0;
      return;
    }
  }
  fprintf(stderr, "No client socket descriptor found");
}

void handle_new_conn(int sock, struct pollfd *fds, uint8_t *fds_count) {
  struct sockaddr_in client_addr;
  socklen_t client_adrr_len = sizeof(client_addr);
  char remote_addr[INET_ADDRSTRLEN];
  int remote_port = 0;

  // accept new client connection
  int sock_client =
      accept(sock, (struct sockaddr *)&client_addr, &client_adrr_len);
  if (sock_client < 0) {
    perror("Accept connection failed");
    return;
  }

  // add client socket to the list of descriptors
  if (add_to_pfds(sock_client, fds, fds_count) == 0) {
    inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr, INET_ADDRSTRLEN);
    fprintf(stdout, "Accpet incoming connection from %s:%d\n", remote_addr,
            remote_port);
  } else {
    fprintf(stderr, "Maximum number of connections has reached");
    close(sock_client);
  }
}

void broadcast(struct pollfd *fds, char *msg, int sock_from) {
  int bytes_sent = 0;
  for (uint8_t i = 1; i < MAX_CLIENT; i++) {
    if (fds[i].fd >= 0 && fds[i].fd != sock_from) {
      if ((bytes_sent = send(fds[i].fd, msg, sizeof(msg), 0)) < 0) {
        perror("Sending message failed");
      }
    }
  }
}

void handle_client_data(struct pollfd *fds, uint8_t fd) {
  char buffer[1024];
  int bytes_read = 0;

  bzero(buffer, 1024);
  if ((bytes_read = recv(fds[fd].fd, &buffer, sizeof(buffer) - 1, 0)) <= 0) {
    // connection closed
    if (bytes_read == 0) {
      fprintf(stdout, "Connection closed by peer");
    }

    // connection error
    if (bytes_read < 0) {
      perror("Receiving data failed");
    }

    close(fds[fd].fd);
    del_from_pfds(fds[fd].fd, fds);
  } else {
    fprintf(stdout, "%s", buffer);
    broadcast(fds, buffer, fds[fd].fd);
  }
}

void process_conn(int server_sock, struct pollfd *fds, u_int8_t *fds_count) {
  // check server fd
  if (fds[0].revents & POLLIN) {
    handle_new_conn(server_sock, fds, fds_count);
  }

  // check other fds
  for (uint8_t i = 1; i < MAX_CLIENT; i++) {
    if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
      handle_client_data(fds, i);
    }
  }
}

int main(int argc, char **argv) {
  uint16_t port = 8012;

  if (argc < 2) {
    usage();
    return 1;
  }
  // TODO: argument parsing needs to be fixed
  if (strcmp("-p", argv[1]) == 0 || strcmp("--port", argv[1]) == 0) {
    port = atoi(argv[2]);
  }
  printf("Starting server on port %d...\n", port);

  // create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  struct sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  // bind socket
  int bnd = bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (bnd < 0) {
    perror("Bind socket failed");
    return 2;
  }

  // listen incoming connections
  int lst = listen(sock, 2);
  if (lst < 0) {
    perror("Listen socket failed");
    return 3;
  }

  char *buffer = "DATA\0";

  struct pollfd fds_arr[MAX_CLIENT];
  for (uint8_t i = 0; i < MAX_CLIENT; i++) {
    fds_arr[i].fd = -1;
    fds_arr[i].events = 0;
    fds_arr[i].revents = 0;
  }

  fds_arr[0].fd = sock;
  fds_arr[0].events = POLLIN;
  fds_arr[0].revents = 0;
  uint8_t fds_count = 1; // server fd

  for (;;) {
    int poll_count = poll(fds_arr, MAX_CLIENT, -1);
    if (poll_count < 0) {
      perror("Poll failed.");
      break;
    }

    process_conn(sock, fds_arr, &fds_count);
  }

  // send(sock_client, buffer, strlen(buffer), 0);

  fprintf(stdout, "Shuting down the server");

  // close server socket
  close(sock);

  return 0;
}
