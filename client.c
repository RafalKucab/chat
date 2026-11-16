#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

int close_conn(int socket) { return close(socket); }

int main() {
  printf("Starting HTTP client...\n");

  char buffer[1024] = {0};
  const int PORT = 8012;
  char *http_request =
      "GET / HTTP/1.1\r\nHost: 142.250.109.101\r\nConnection: close\r\n\r\n";

  int sock = open_conn("127.0.0.1", PORT);
  send(sock, http_request, strlen(http_request), 0);

  int bytes_read = 0;
  while ((bytes_read = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytes_read] = '\0';
    printf("%s\n", buffer);
  }

  if (bytes_read < 0) {
    perror("Reading from socket failed");
  }

  // close the socket
  close_conn(sock);

  return 0;
}
