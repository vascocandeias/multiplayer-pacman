#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

// gcc teste.c UI_library.c -o teste-UI -lSDL2 -lSDL2_image

#include "UI_library.h"
#include "message.h"

#define PORT 3000
#define WIDTH 30
#define HEIGHT 10
#define SOCKET_LEN 100

int remote_fd = 0;
int fd;
int server;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
socklen_t size_addr;

// this variable will contain the identifier for our own event type
Uint32 Event_ShowUser;

// change to use list -> move to server

int sockets[SOCKET_LEN];
int n_sockets = 0;

void insert_socket(int fd) { sockets[n_sockets++] = fd; }

void send_messages(message msg) {
  for (int i = 0; i < n_sockets; ++i) {
    write(sockets[i], &msg, sizeof(msg));
  }
}

void* thread_user(void* arg) {
  message m;
  message* event_data;
  int color[3];
  int err_rcv;
  int fd = *((int*)arg);
  SDL_Event new_event;

  insert_socket(fd);

  read(fd, &m, sizeof(m));

  color[0] = m.color[0];
  color[1] = m.color[1];
  color[2] = m.color[2];

  m.x = WIDTH;
  m.y = HEIGHT;
  m.id = fd;

  write(fd, &m, sizeof(m));

  while ((err_rcv = read(fd, &m, sizeof(m))) > 0) {
    printf("received: %d %d - %d %d %d - old - %d %d\n", getpid(), err_rcv,
           m.character, m.x, m.y, m.old_x, m.old_y);

    if (m.x == m.old_x && m.y == m.old_y) continue;

    // create the data that will contain the new lemon position
    event_data = malloc(sizeof(message));
    if (!event_data) {
      perror("malloc");
      exit(-1);
    }
    *event_data = m;

    // clear the event data
    SDL_zero(new_event);
    // define event type
    new_event.type = Event_ShowUser;
    // assign the event data
    new_event.user.data1 = event_data;
    // send the event
    SDL_PushEvent(&new_event);
  }
  perror("read");
  remote_fd = 0;
  return NULL;
}

void* thread_accept(void* arg) {
  int remote_fd;
  while (1) {
    printf("%d Ready to accept connections\n", getpid());
    remote_fd = accept(fd, (struct sockaddr*)&remote_addr, &size_addr);
    if (remote_fd == -1) {
      perror("accept");
      exit(-1);
    }
    printf("accepted connection\n");
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_user, &remote_fd);
  }
}

int main(int argc, char* argv[]) {
  SDL_Event event;
  int done = 0;
  message m;
  Event_ShowUser = SDL_RegisterEvents(1);

  // creates a windows and a board with 50x20 cases
  create_board_window(WIDTH, HEIGHT);

  if (argc == 1) {
    server = 1;
    printf("server\n");

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
      perror("socket");
      exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(PORT);

    printf("port: %d\n", PORT);

    if (bind(fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
      perror("bind");
      exit(-1);
    }

    printf(" socket created and binded \n");

    if (listen(fd, 2) == -1) {
      perror("listen");
      exit(-1);
    }
  } else
    exit(0);

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, thread_accept, NULL);
  // monster and packman position
  int x = 0;
  int y = 0;
  int other_x = 0;
  int other_y = 0;
  // variable that defines what color to paint the monstes

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }
      if (event.type == Event_ShowUser) {
        // we get the data (created with the malloc)
        message* data = event.user.data1;
        printf("data: %d %d %d %d %d %d\n", data->old_x, data->old_y, data->x,
               data->y, data->character, data->id);
        if (data->x == data->old_x && data->y == data->old_y) printf("same\n");
        // retrieve the x and y printf("before clear\n");
        clear_place(data->old_x, data->old_y);
        other_x = data->x;
        other_y = data->y;
        switch (data->character) {
          case PACMAN:
            paint_pacman(other_x, other_y, data->color[0], data->color[1],
                         data->color[2]);
            break;
          case MONSTER:
            paint_monster(other_x, other_y, data->color[0], data->color[1],
                          data->color[2]);
            break;
        }
        send_messages(*data);
        free(data);
      }
    }
  }
  close(fd);
  close(remote_fd);

  printf("fim\n");
  close_board_windows();
}
