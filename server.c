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

void init_player(int fd, player* p, place* pac, place* mon) {
  int color[3];
  message m;
  message* event_data;
  int pos[2];
  SDL_Event new_event;

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
  players = insert_player(players, p, fd);
  printf("inserted %d\n", fd);

  send_board(board, fd, width, height);

  memcpy(m.color, color, sizeof(color));
  m.old_x = -1;
  m.old_y = -1;
  m.id = fd;

  random_character(board, width, height, pac, fd, color, PACMAN, pos);
  set_pacman(players, fd, pos);

  m.type = PACMAN;
  m.x = pos[0];
  m.y = pos[1];
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

  random_character(board, width, height, mon, fd, color, MONSTER, pos);
  printf("monster going to %d %d\n", pos[0], pos[1]);
  set_monster(players, fd, pos);
  get_monster(players, fd);

  m.type = MONSTER;
  m.x = pos[0];
  m.y = pos[1];

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
  printf("init end %d\n", fd);
  get_monster(players, fd);
}

void* thread_user(void* arg) {
  message m;
  message* event_data;
  int err_rcv;
  int fd = *(int*)arg;
  SDL_Event new_event;
  int* type;

  player p;
  place pac, mon;
  init_player(fd, &p, &pac, &mon);

  printf("after init %d\n", fd);
  get_monster(players, fd);

  while ((err_rcv = read(fd, &m, sizeof(m))) > 0) {
    if ((abs(m.x) + abs(m.y)) != 1) continue;

    place* aux;

    switch (m.type) {
      case PACMAN:
        type = get_pacman(players, fd);
        aux = get_place(board, type);
        break;
      case MONSTER:
        type = get_monster(players, fd);
        aux = get_place(board, type);
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
    if (m.x == -1 || m.x == width ||
        (board[m.x][m.y] && board[m.x][m.y]->type == BRICK))
      m.x -= 2 * aux_x;
    if (m.y == -1 || m.y == height ||
        (board[m.x][m.y] && board[m.x][m.y]->type == BRICK))
      m.y -= 2 * aux_y;

    if (m.x < 0 || m.x >= width || m.y < 0 || m.y >= height) continue;

    if (board[m.x][m.y]) {
      switch (board[m.x][m.y]->type) {
        case BRICK:
        case PACMAN:
          continue;
        case MONSTER:
          if (m.type == MONSTER || board[m.x][m.y]->id == fd) {
            message aux;

            printf("move monster into monster or pacman into same monster\n");

            aux.id = board[m.x][m.y]->id;

            aux.type = board[m.x][m.y]->type;
            aux.old_x = -1;
            aux.old_y = -1;
            aux.x = m.old_x;
            aux.y = m.old_y;

            int pos[] = {aux.x, aux.y};
            set_monster(players, aux.id, pos);
            m.old_x = -1;
            m.old_y = -1;

            memcpy(aux.color, board[m.x][m.y]->color, sizeof(aux.color));
            board[aux.x][aux.y] = board[m.x][m.y];

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
          board[type[0]][type[1]] = NULL;
          break;
      }
    } else
      board[type[0]][type[1]] = NULL;

    board[m.x][m.y] = aux;
    type[0] = m.x;
    type[1] = m.y;

    switch (m.type) {
      case PACMAN:
        set_pacman(players, fd, type);
        break;
      case MONSTER:
        set_monster(players, fd, type);
      default:
        break;
    }

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

  printf("client %d disconnected\n", fd);
  close(fd);
  type = get_monster(players, fd);
  board[type[0]][type[1]] = NULL;

  m.type = CLEAR;
  m.old_x = type[0];
  m.old_y = type[1];
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

  type = get_pacman(players, fd);
  board[type[0]][type[1]] = NULL;

  m.old_x = type[0];
  m.old_y = type[1];

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
  delete_node(players, fd);

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
