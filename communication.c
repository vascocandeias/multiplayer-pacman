#include "communication.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fruits.h"
#include "list.h"
#include "message.h"
#include "players.h"

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

void* thread_accept(void* arg) {
  struct sockaddr_in remote_addr;
  socklen_t size_addr;
  character aux[2] = {CHERRY, LEMON};
  int n_players;

  int remote_fd;
  int fd = *(int*)arg;
  while (1) {
    printf("%d Ready to accept connections\n", getpid());
    remote_fd = accept(fd, (struct sockaddr*)&remote_addr, &size_addr);
    if (remote_fd == -1) {
      perror("accept");
      exit(-1);
    }
    printf("accepted connection\n");
    if (!has_room()) {
      close(remote_fd);
      continue;
    }
    n_players = get_n_players();
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_user, &remote_fd);
    if (n_players) {
      // create fruits
      for (int i = 0; i < 2; ++i)
        pthread_create(&thread_id, NULL, thread_fruit, &aux[i]);
    }
  }
}
