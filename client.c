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

int remote_fd = 0;
int fd;
int server;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
socklen_t size_addr;
int width = 50;
int height = 20;

// this variable will contain the identifier for our own event type
Uint32 Event_ShowUser;

void* thread_user(void* arg) {
  message m;
  message* event_data;
  int err_rcv;
  SDL_Event new_event;

  while ((err_rcv = read(remote_fd, &m, sizeof(m))) > 0) {
    printf("received: %d %d - %d %d %d\n", getpid(), err_rcv, m.type, m.x, m.y);

    // create the data that will contain the new lemon position
    event_data = malloc(sizeof(message));
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
  return NULL;
}

int main(int argc, char* argv[]) {
  SDL_Event event;
  int done = 0;
  int id = 0;
  message m, msg;
  Event_ShowUser = SDL_RegisterEvents(1);

  if (argc != 6) exit(-1);

  remote_fd = socket(AF_INET, SOCK_STREAM, 0);
  server = 0;
  printf("client\n");

  if (fd == -1) {
    perror("socket: ");
    exit(-1);
  }

  printf(" socket created \n");

  remote_addr.sin_family = AF_INET;
  int port;
  sscanf(argv[2], "%d", &port);
  remote_addr.sin_port = htons(port);
  inet_aton(argv[1], &remote_addr.sin_addr);

  if (-1 == connect(remote_fd, (const struct sockaddr*)&remote_addr,
                    sizeof(remote_addr))) {
    perror("connect");
    exit(-1);
  }

  printf("connected\n");

  sscanf(argv[3], "%d", &m.color[0]);
  sscanf(argv[4], "%d", &m.color[1]);
  sscanf(argv[5], "%d", &m.color[2]);

  printf("sending init\n");

  if (remote_fd) write(remote_fd, &m, sizeof(m));

  if (read(remote_fd, &m, sizeof(m)) <= 0) {
    perror("receiving board size");
  }

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, thread_user, NULL);

  width = m.x;
  height = m.y;
  id = m.id;

  // creates a windows and a board with 50x20 cases
  create_board_window(width, height);

  // monster and pacman position
  int pacman[2];
  pacman[0] = 0;
  pacman[1] = 0;
  int monster[2];
  monster[0] = 0;
  monster[1] = 0;
  // variable that defines what color to paint the monstes

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }

      // when the mouse mooves the monster also moves
      if (event.type == SDL_MOUSEMOTION) {
        int x_new, y_new;
        // this fucntion return the place cwher the mouse cursor is
        get_board_place(event.motion.x, event.motion.y, &x_new, &y_new);
        // if the mluse moved toi anothe place
        if ((x_new != pacman[0]) || (y_new != pacman[1])) {
          m.x = x_new - pacman[0];
          m.y = y_new - pacman[1];
          // m.old_x = pacman[0];
          // m.old_y = pacman[1];
          m.type = PACMAN;

          if (remote_fd) write(remote_fd, &m, sizeof(m));
          // printf("move pacman x-%d y-%d\n", m.x, m.y);
        }
      }
      if (event.type == SDL_KEYDOWN) {
        m.x = 0;
        m.y = 0;
        m.type = MONSTER;
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            if (monster[0] >= 0) m.x--;
            break;
          case SDLK_RIGHT:
            if (monster[0] < width) m.x++;
            break;
          case SDLK_UP:
            if (monster[1] >= 0) m.y--;
            break;
          case SDLK_DOWN:
            if (monster[1] < height) m.y++;
            break;
          default:
            break;
        }
        if (!m.x && !m.y) continue;
        if (remote_fd) write(remote_fd, &m, sizeof(m));
        printf("moving monster x-%d y-%d\n", m.x, m.y);
      }
      if (event.type == Event_ShowUser) {
        // we get the data (created with the malloc)
        message* data = event.user.data1;
        // retrieve the x and y
        printf("move: %d %d %d -- %d %d\n", data->type, data->old_x,
               data->old_y, data->x, data->y);
        if (data->old_x != -1 && data->old_y != -1)
          clear_place(data->old_x, data->old_y);
        switch (data->type) {
          case PACMAN:
            printf("print pacman\n");
            paint_pacman(data->x, data->y, data->color[0], data->color[1],
                         data->color[2]);
            break;
          case MONSTER:
            printf("print monster\n");
            paint_monster(data->x, data->y, data->color[0], data->color[1],
                          data->color[2]);
            printf("monster printed\n");
            break;
          case BRICK:
            paint_brick(data->x, data->y);
          default:
            break;
        }
        if (data->id == id) {
          switch (data->type) {
            case PACMAN:
              pacman[0] = data->x;
              pacman[1] = data->y;
              break;
            case MONSTER:
              monster[0] = data->x;
              monster[1] = data->y;
              break;
            default:
              break;
          }
        }
        free(data);
      }
    }
  }
  close(fd);
  close(remote_fd);

  printf("fim\n");
  close_board_windows();
}
