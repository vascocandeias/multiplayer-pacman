#include "board.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "UI_library.h"
#include "fruits.h"
#include "list.h"
#include "message.h"
#include "players.h"
#include "pthread.h"

#define MAX_LEN 128

static gameboard board;
static place* brick;
static int columns = 30, rows = 10;
static int free_places = 0;
static pthread_mutex_t** mutex_board;

void create_board() {
  board = (gameboard)malloc(columns * sizeof(place**));
  mutex_board = (pthread_mutex_t**)malloc(columns * sizeof(pthread_mutex_t*));

  if (!board || !mutex_board) {
    perror("memory");
    exit(-1);
  }
  free_places = columns * rows;
  for (int i = 0; i < columns; ++i) {
    board[i] = (place**)malloc(rows * sizeof(place*));
    mutex_board[i] = (pthread_mutex_t*)malloc(rows * sizeof(pthread_mutex_t));
    if (!board[i] || !mutex_board[i]) {
      perror("memory");
      exit(-1);
    }
    for (int j = 0; j < rows; ++j) {
      board[i][j] = NULL;
      pthread_mutex_init(&mutex_board[i][j], NULL);
    }
  }
}

void init_board(char* filename) {
  char buffer[MAX_LEN];
  FILE* file = NULL;
  brick = malloc(sizeof(place*));

  brick->id = -1;
  brick->type = BRICK;

  if ((file = fopen(filename, "r"))) {
    if (fgets(buffer, MAX_LEN, file)) {
      int aux1, aux2;
      if (sscanf(buffer, "%d %d", &aux1, &aux2) == 2) {
        columns = aux1;
        rows = aux2;
        create_board();
        create_board_window(columns, rows);
        int row = 0;
        while (fgets(buffer, MAX_LEN, file)) {
          for (int column = 0; column < strlen(buffer); ++column)
            if (buffer[column] == 'B') {
              board[column][row] = brick;
              paint_brick(column, row);
              free_places--;
            }
          row++;
        }
      }
    }
  }
  if (!board) {
    create_board();
    create_board_window(columns, rows);
  }
}

bool has_room() { return free_places >= 4 * get_n_players() + 2; }

void delete_board() {
  for (int i = 0; i < columns; ++i) {
    free(board[i]);
    free(mutex_board[i]);
  }
  free(board);
  free(mutex_board);
  free(brick);
}

int random_position(place* p, int pos[2]) {
  pos[0] = random() % columns;
  pos[1] = random() % rows;

  pthread_mutex_lock(&mutex_board[pos[0]][pos[1]]);
  while (board[pos[0]][pos[1]]) {
    pthread_mutex_unlock(&mutex_board[pos[0]][pos[1]]);
    printf("trying y=%d type=%d id=%d\n", pos[1], p->type, p->id);
    pos[0] = random() % columns;
    pos[1] = random() % rows;
  };
  board[pos[0]][pos[1]] = p;
  switch (p->type) {
    case MONSTER:
      set_monster(p->id, pos);
      break;
    case POWER:
    case PACMAN:
      set_pacman(p->id, pos);
      break;
    default:
      break;
  }
  pthread_mutex_unlock(&mutex_board[pos[0]][pos[1]]);

  return 1;
}

void send_board(int fd) {
  message m;
  place aux;

  for (int j = 0; j < columns; ++j) {
    for (int i = 0; i < rows; ++i) {
      pthread_mutex_lock(&mutex_board[j][i]);
      if (board[j][i]) {
        aux = *board[j][i];
        pthread_mutex_unlock(&mutex_board[j][i]);
        m.id = aux.id;
        m.type = aux.type;
        m.x = j;
        m.y = i;
        m.old_x = m.x;
        m.old_y = m.y;
        memcpy(m.color, aux.color, sizeof(m.color));
        write(fd, &m, sizeof(m));
      } else
        pthread_mutex_unlock(&mutex_board[j][i]);
    }
  }
}

place* get_place(int position[2]) { return board[position[0]][position[1]]; }

void random_character(place* p, int id, int color[3], character c,
                      int position[2]) {
  memcpy(p->color, color, sizeof(int) * 3);
  p->id = id;
  p->type = c;

  random_position(p, position);
}

void handle_request(message this, int id) {
  int aux_x, aux_y;
  int old_pos[2];
  int new_pos[2];
  place* old_place;
  place* new_place;
  place new;
  character aux;

  // printf("handle type %d x: %d y: %d\n", this.type, this.x, this.y);

  while (1) {
    if (this.type == PACMAN || this.type == POWER)
      memcpy(old_pos, get_pacman(id), sizeof(old_pos));
    else if (this.type == MONSTER)
      memcpy(old_pos, get_monster(id), sizeof(old_pos));
    else  // not possible
      return;

    this.old_x = old_pos[0];
    this.old_y = old_pos[1];
    aux_x = this.x;
    aux_y = this.y;
    new_pos[0] = old_pos[0] + aux_x;
    new_pos[1] = old_pos[1] + aux_y;

    // if the player moves horizontally out of bounds
    if (new_pos[0] == -1 || new_pos[0] == columns) new_pos[0] -= 2 * aux_x;
    // if the player moves vertically out of bounds
    else if (new_pos[1] == -1 || new_pos[1] == rows)
      new_pos[1] -= 2 * aux_y;
    // if the player moves into a brick
    else {
      pthread_mutex_lock(&mutex_board[new_pos[0]][new_pos[1]]);
      new_place = get_place(new_pos);
      aux = new_place ? new_place->type : CLEAR;
      pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);
      if (aux == BRICK) {
        new_pos[0] -= 2 * aux_x;
        new_pos[1] -= 2 * aux_y;
      }
    }

    // if the bounce got him out of bounds
    if (new_pos[0] < 0 || new_pos[0] >= columns || new_pos[1] < 0 ||
        new_pos[1] >= rows)
      return;

    this.x = new_pos[0];
    this.y = new_pos[1];

    // get board places
    // order to void deadlock
    if (old_pos[0] < new_pos[0] || old_pos[1] < new_pos[1]) {
      // lock old first
      pthread_mutex_lock(&mutex_board[old_pos[0]][old_pos[1]]);
      old_place = get_place(old_pos);
      // check if the player is still there
      if (old_place->id != id || old_place->type % POWER != this.type % POWER) {
        pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);
        continue;
      }
      // lock new next
      pthread_mutex_lock(&mutex_board[new_pos[0]][new_pos[1]]);
      new_place = get_place(new_pos);

    } else {
      // lock old first
      pthread_mutex_lock(&mutex_board[new_pos[0]][new_pos[1]]);
      new_place = get_place(new_pos);
      // lock new next
      pthread_mutex_lock(&mutex_board[old_pos[0]][old_pos[1]]);
      old_place = get_place(old_pos);
      // check if the player is still there
      if (old_place->id != id || old_place->type % POWER != this.type % POWER) {
        pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);
        pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);
        continue;
      }
    }

    // ensure this.type is coherent with real type (because of powered pacmen)
    this.type = old_place->type;

    if (new_place) {
      switch (new_place->type) {
        case BRICK:
          break;
        case PACMAN:
          if (old_place->type == PACMAN || new_place->id == id) {
            // swap
            board[new_pos[0]][new_pos[1]] = old_place;
            set_character(old_place->type, old_place->id, new_pos);
            pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);

            board[old_pos[0]][old_pos[1]] = new_place;
            set_character(new_place->type, new_place->id, old_pos);
            new = *new_place;
            pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);

            draw_swap(this, new, old_pos);
            return;
          } else
            break;
        case MONSTER:
          if (old_place->type == MONSTER || new_place->id == id) {
            // swap
            board[new_pos[0]][new_pos[1]] = old_place;
            set_character(old_place->type, old_place->id, new_pos);
            pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);

            board[old_pos[0]][old_pos[1]] = new_place;
            set_character(new_place->type, new_place->id, old_pos);
            new = *new_place;
            pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);

            draw_swap(this, new, old_pos);
            return;
          } else
            break;
          break;
        case CHERRY:
        case LEMON:
          board[old_pos[0]][old_pos[1]] = NULL;
          pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);

          if (old_place->type == PACMAN) old_place->type = POWER;
          board[new_pos[0]][new_pos[1]] = old_place;
          set_character(old_place->type, id, new_pos);
          pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);

          if (old_place->type == POWER) this.type = POWER;
          eat_fruit(new_place->id);
          draw_character(this);
        default:
          // if it's a power pacman
          break;
      }
    } else {
      board[old_pos[0]][old_pos[1]] = NULL;
      pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);

      board[new_pos[0]][new_pos[1]] = old_place;
      set_character(this.type, id, new_pos);
      pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);
      draw_character(this);
      return;
    }
    pthread_mutex_unlock(&mutex_board[new_pos[0]][new_pos[1]]);
    pthread_mutex_unlock(&mutex_board[old_pos[0]][old_pos[1]]);
    return;
  }
}

int delete_place(int position[2], int id, character c) {
  place* p = NULL;
  pthread_mutex_lock(&mutex_board[position[0]][position[1]]);
  p = board[position[0]][position[1]];
  if (!p || p->id != id || p->type % POWER != c % POWER) {
    pthread_mutex_unlock(&mutex_board[position[0]][position[1]]);
    return 0;
  }
  board[position[0]][position[1]] = NULL;
  pthread_mutex_unlock(&mutex_board[position[0]][position[1]]);
  return 1;
}

int get_columns() { return columns; }

int get_rows() { return rows; }

void swap(place* new_place, place* old_place, int new_pos[2], int old_pos[2]) {
  board[old_pos[0]][old_pos[1]] = new_place;
  set_character(new_place->type, new_place->id, old_pos);
  board[new_pos[0]][new_pos[1]] = old_place;
  set_character(old_place->type, old_place->id, new_pos);
}

void draw_swap(message m, place new_place, int old_pos[2]) {
  message other;
  other.id = new_place.id;
  other.type = new_place.type;
  other.old_x = -1;
  other.old_y = -1;
  other.x = old_pos[0];
  other.y = old_pos[1];
  memcpy(other.color, new_place.color, sizeof(other.color));

  m.old_x = -1;
  m.old_y = -1;

  draw_character(other);
  draw_character(m);
}
