/*****************************************************************************
 * File name: board.h
 *
 *  Author: Vasco Candeias nº 84196
 *
 *  Release date: 29/04/2020
 *
 *  Description: Headers file for the board.c functions
 *
 ****************************************************************************/

#include <stdlib.h>

#ifndef BOARD_H
#define BOARD_H

typedef enum character {
  CLEAR,
  PACMAN,
  MONSTER,
  POWER,
  BRICK,
  LEMON,
  CHERRY,
} character;

typedef struct place {
  character type;
  int id;
  // int color[3];
  int* color;
} place;

#define DEFAULT_PLACE \
  { CLEAR, -1, NULL }

typedef place** gameboard;

gameboard create_board(int columns, int rows);
gameboard init_board(int* width, int* height, char* filename);
int random_position(gameboard board, int* pos, int width, int height);
void send_board(gameboard board, int fd, int width, int height);

#endif
