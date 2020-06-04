/*
 * file: server.c
 * author: Vasco Candeias (vascocandeias@tecnico.ulisboa.pt)
 * description: main server application to control the multiplayer pacman game
 * date: 01-06-2020
 */

#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "UI_library.h"
#include "board.h"
#include "communication.h"
#include "fruits.h"
#include "message.h"
#include "players.h"

// define FORCE_RENDERER to force board rendering in case some things are not
// appearing right away

// #define FORCE_RENDER
#define FILENAME "board.txt"
#define PORT 3000

int fd;

// Signal handling closes sockets and deletes semaphores
void terminate(int signal) {
  printf("\nExiting due to signal\n");
  delete_fruits();
  delete_players();
  close(fd);
  close_board_windows();
  exit(0);
}

int main(int argc, char* argv[]) {
  SDL_Event event;
  Uint32 Event_ShowUser;
  int done = 0;
  Event_ShowUser = SDL_RegisterEvents(1);

  srandom(time(NULL));

  if (argc > 2) exit(-1);

  printf("server\n");

  fd = init_server(PORT);

  if (signal(SIGINT, terminate) == SIG_ERR ||
      signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    perror("signal handlers");

  int free_places;
  if (argc == 2)
    free_places = init_board(argv[1]);
  else
    free_places = init_board(FILENAME);

  int max_players = (free_places + 2) / 4;
  int max_fruits = (max_players - 1) * 2;

  init_players(max_players, Event_ShowUser);
  init_fruits(max_fruits);

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, thread_accept, &fd);

  while (!done) {
#ifdef FORCE_RENDER
    render();
#endif
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }
      if (event.type == Event_ShowUser) {
        message* data = event.user.data1;

        switch (data->type) {
          case CLEAR:
            clear_place(data->x, data->y);
            break;
          case PACMAN:
            paint_pacman(data->x, data->y, data->color[0], data->color[1],
                         data->color[2]);
            break;
          case MONSTER:
            paint_monster(data->x, data->y, data->color[0], data->color[1],
                          data->color[2]);
            break;
          case POWER:
            paint_powerpacman(data->x, data->y, data->color[0], data->color[1],
                              data->color[2]);
            break;
          case LEMON:
            paint_lemon(data->x, data->y);
            break;
          case CHERRY:
            paint_cherry(data->x, data->y);
            break;
          default:
            break;
        }
        free(data);
      }
    }
  }
  delete_fruits();
  delete_players();
  close(fd);
  close_board_windows();
}
