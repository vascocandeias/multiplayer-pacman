#include "board.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "UI_library.h"
#include "message.h"

#define MAX_LEN 128

gameboard create_board(int columns, int rows) {
  gameboard board;
  board = malloc(columns * sizeof(place*));
  place init = DEFAULT_PLACE;
  if (!board) {
    perror("memory");
    exit(-1);
  }

  for (int i = 0; i < columns; ++i) {
    board[i] = malloc(rows * sizeof(place));
    if (!board[i]) {
      perror("memory");
      exit(-1);
    }
    for (int j = 0; j < rows; ++j) memcpy(&board[i][j], &init, sizeof(init));
  }

  return board;
}

gameboard init_board(int* width, int* height, char* filename) {
  gameboard board = NULL;
  char buffer[MAX_LEN];
  FILE* file = NULL;

  if ((file = fopen(filename, "r"))) {
    if (fgets(buffer, MAX_LEN, file)) {
      int aux1, aux2;
      if (sscanf(buffer, "%d %d", &aux1, &aux2) == 2) {
        *width = aux1;
        *height = aux2;
        board = create_board(*width, *height);
        create_board_window(*width, *height);
        int row = 0;
        while (fgets(buffer, MAX_LEN, file)) {
          for (int column = 0; column < strlen(buffer); ++column)
            if (buffer[column] == 'B') {
              board[column][row].type = BRICK;
              paint_brick(column, row);
            }
          row++;
        }
      }
    }
  }

  if (!board) {
    board = create_board(*width, *height);
    create_board_window(*width, *height);
  }
  return board;
}

int random_position(gameboard board, int* pos, int width, int height) {
  do {
    pos[0] = random() % width;
    pos[1] = random() % height;
  } while (board[pos[0]][pos[1]].type);
  return 1;
}

void send_board(gameboard board, int fd, int width, int height) {
  message m;

  for (int j = 0; j < width; ++j) {
    for (int i = 0; i < height; ++i) {
      if (board[j][i].type) {
        m.id = board[j][i].id;
        m.type = board[j][i].type;
        m.x = j;
        m.y = i;
        m.old_x = m.x;
        m.old_y = m.y;
        if (board[j][i].color)
          memcpy(m.color, board[j][i].color, sizeof(m.color));
        write(fd, &m, sizeof(m));
      }
    }
  }
}