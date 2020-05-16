#include "communication.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list.h"
#include "message.h"

int init_server(int port) {
  struct sockaddr_in local_addr;

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd == -1) {
    perror("socket");
    exit(-1);
  }

  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(port);

  printf("port: %d\n", port);

  if (bind(fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
    perror("bind");
    exit(-1);
  }

  printf(" socket created and binded \n");

  if (listen(fd, 2) == -1) {
    perror("listen");
    exit(-1);
  }

  return fd;
}

