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
  int color[3];
} place;

#define BRICK_PLACE \
  { BRICK, -1, NULL }

typedef place*** gameboard;

gameboard create_board(int columns, int rows);
gameboard init_board(int* width, int* height, char* filename);
int random_position(gameboard board, int width, int height, place* p,
                    int position[2]);
void send_board(gameboard board, int fd, int width, int height);
place* get_place(gameboard board, int position[2]);
void random_character(gameboard board, int width, int height, place* p, int id,
                      int color[3], character c, int position[2]);

#endif
