#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

// gcc teste.c UI_library.c -o teste-UI -lSDL2 -lSDL2_image

#include "UI_library.h"
#include "board.h"
#include "communication.h"
#include "list.h"
#include "message.h"
#include "players.h"

#define FILENAME "board.txt"
#define PORT 3000
#define MAX_LEN 128

int width = 30, height = 10;
int remote_fd = 0;
struct sockaddr_in remote_addr;
socklen_t size_addr;

gameboard board;

// this variable will contain the identifier for our own event type
Uint32 Event_ShowUser;

List players;

void* thread_user(void* arg) {
  message m;
  message* event_data;
  int color[3];
  int err_rcv;
  int fd = *(int*)arg;
  SDL_Event new_event;
  // monster and pacman position
  int pacman[2];
  pacman[0] = 0;
  pacman[1] = 0;
  int monster[2];
  monster[0] = 0;
  monster[1] = 0;
  int* type;

  read(fd, &m, sizeof(m));

  for (int i = 0; i < 3; ++i) {
    if (m.color[i] > 255)
      m.color[i] = 255;
    else if (m.color[i] < 0)
      m.color[i] = 0;

    color[i] = m.color[i];
  }

  m.x = width;
  m.y = height;
  m.id = fd;

  write(fd, &m, sizeof(m));
  player p;
  players = insert_player(players, p, fd);

  send_board(board, fd, width, height);

  random_position(board, pacman, width, height);
  board[pacman[0]][pacman[1]].type = PACMAN;
  board[pacman[0]][pacman[1]].id = fd;
  board[pacman[0]][pacman[1]].color = color;
  // memcpy(board[pacman[0]][pacman[1]].color, color, sizeof(color));

  memcpy(m.color, color, sizeof(color));
  m.id = fd;
  m.type = PACMAN;
  m.x = pacman[0];
  m.y = pacman[1];
  m.old_x = -1;
  m.old_y = -1;
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

  random_position(board, monster, width, height);
  board[monster[0]][monster[1]].type = MONSTER;
  board[monster[0]][monster[1]].id = fd;
  // memcpy(board[monster[0]][monster[1]].color, color, sizeof(color));
  board[monster[0]][monster[1]].color = color;

  m.type = MONSTER;
  m.x = monster[0];
  m.y = monster[1];
  m.old_x = -1;
  m.old_y = -1;

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

  while ((err_rcv = read(fd, &m, sizeof(m))) > 0) {
    if ((abs(m.x) + abs(m.y)) != 1) continue;

    switch (m.type) {
      case PACMAN:
        type = pacman;
        break;
      case MONSTER:
        type = monster;
      default:
        break;
    }

    m.old_x = type[0];
    m.old_y = type[1];
    int aux_x = m.x;
    int aux_y = m.y;
    m.x = type[0] + m.x;
    m.y = type[1] + m.y;
    if (m.x < -1 || m.x > width || m.y < -1 || m.y > height) continue;
    if (m.x == -1 || m.x == width || board[m.x][m.y].type == BRICK)
      m.x -= 2 * aux_x;
    if (m.y == -1 || m.y == height || board[m.x][m.y].type == BRICK)
      m.y -= 2 * aux_y;

    if (m.x < 0 || m.x >= width || m.y < 0 || m.y >= height) continue;

    switch (board[m.x][m.y].type) {
      case BRICK:
      case PACMAN:
        continue;
      case MONSTER:
        if (m.type == MONSTER || board[m.x][m.y].id == fd) {
          message aux;

          printf("move monster into monster or pacman into same monster\n");

          aux.id = board[m.x][m.y].id;

          aux.type = board[m.x][m.y].type;
          aux.old_x = -1;
          aux.old_y = -1;
          aux.x = m.old_x;
          aux.y = m.old_y;
          if (aux.id == fd) {
            monster[0] = aux.x;
            monster[1] = aux.y;
          }
          m.old_x = -1;
          m.old_y = -1;

          memcpy(aux.color, board[m.x][m.y].color, sizeof(aux.color));

          memcpy(&board[aux.x][aux.y], &board[m.x][m.y],
                 sizeof(board[m.x][m.y]));

          // create the data that will contain the new lemon position
          event_data = malloc(sizeof(message));
          if (!event_data) {
            perror("malloc");
            exit(-1);
          }
          *event_data = aux;

          // clear the event data
          SDL_zero(new_event);
          // define event type
          new_event.type = Event_ShowUser;
          // assign the event data
          new_event.user.data1 = event_data;
          // send the event
          SDL_PushEvent(&new_event);
        } else
          continue;
        break;
      default:
        board[type[0]][type[1]].type = CLEAR;
        board[type[0]][type[1]].id = -1;
        break;
    }

    board[m.x][m.y].type = m.type;
    // memcpy(board[m.x][m.y].color, m.color, sizeof(m.color));
    board[m.x][m.y].color = color;
    board[m.x][m.y].id = fd;

    type[0] = m.x;
    type[1] = m.y;

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
  delete_node(players, fd);
  close(fd);
  board[monster[0]][monster[1]].type = CLEAR;
  board[monster[0]][monster[1]].id = -1;
  memset(board[monster[0]][monster[1]].color, 0, sizeof(color));

  m.type = CLEAR;
  m.old_x = monster[0];
  m.old_y = monster[1];
  m.id = -1;

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

  board[pacman[0]][pacman[1]].type = CLEAR;
  board[pacman[0]][pacman[1]].id = -1;
  memset(board[pacman[0]][pacman[1]].color, 0, sizeof(color));

  m.old_x = pacman[0];
  m.old_y = pacman[1];

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

  return NULL;
}

void* thread_accept(void* arg) {
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
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_user, &remote_fd);
  }
}

int main(int argc, char* argv[]) {
  SDL_Event event;
  int done = 0;
  Event_ShowUser = SDL_RegisterEvents(1);

  srandom(time(NULL));

  if (argc != 1) exit(-1);

  printf("server\n");

  int fd = init_server(PORT);

  board = init_board(&width, &height, FILENAME);

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, thread_accept, &fd);

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }
      if (event.type == Event_ShowUser) {
        // we get the data (created with the malloc)
        message* data = event.user.data1;
        // printf("data: %d %d %d %d %d %d\n", data->old_x, data->old_y,
        // data->x,
        //        data->y, data->type, data->id);
        if (data->x == data->old_x && data->y == data->old_y) printf("same\n");
        // retrieve the x and y printf("before clear\n");
        if (data->old_x != -1 && data->old_y != -1)
          clear_place(data->old_x, data->old_y);
        switch (data->type) {
          case PACMAN:
            paint_pacman(data->x, data->y, data->color[0], data->color[1],
                         data->color[2]);
            break;
          case MONSTER:
            paint_monster(data->x, data->y, data->color[0], data->color[1],
                          data->color[2]);
            break;
          default:
            break;
        }
        send_messages(*data, players);
        free(data);
      }
    }
  }
  close(fd);
  close(remote_fd);
  delete_list(players);
  printf("fim\n");
  close_board_windows();
}
