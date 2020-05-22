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
#include "list.h"
#include "message.h"
#include "players.h"

// #define FORCE_RENDER
#define FILENAME "board.txt"
#define PORT 3000

int main(int argc, char* argv[]) {
  SDL_Event event;
  // this variable will contain the identifier for our own event type
  Uint32 Event_ShowUser;
  int done = 0;
  Event_ShowUser = SDL_RegisterEvents(1);

  srandom(time(NULL));

  if (argc > 2) exit(-1);

  printf("server\n");

  int fd = init_server(PORT);
  if (argc == 2)
    init_board(argv[1]);
  else
    init_board(FILENAME);

  init_players(Event_ShowUser);
  init_fruits(5);

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
        // we get the data (created with the malloc)
        message* data = event.user.data1;

        // if (data->x == data->old_x && data->y == data->old_y) printf("same
        // %d\n", data->y); retrieve the x and y printf("before clear\n");
        // printf("printing %d\n", data->type);
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
          case POWER:
            paint_powerpacman(data->x, data->y, data->color[0], data->color[1],
                              data->color[2]);
            // printf("print power!!!\n");
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
        data->score = -1;
        send_messages(*data);
        free(data);
      }
    }
  }
  close(fd);
  delete_players();
  delete_fruits();
  delete_board();
  close_board_windows();
}
