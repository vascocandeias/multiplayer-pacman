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

#include <stdbool.h>
#include <stdlib.h>

#include "UI_library.h"
#include "message.h"

#ifndef BOARD_H
#define BOARD_H

typedef struct place {
  character type;
  int id;
  int color[3];
  int kill_count;
} place;

typedef place*** gameboard;

void create_board();
int init_board(char* filename);
void delete_board();
bool has_room();
int random_position(place* p, int position[2]);
int send_board(int fd);
place* get_place(int position[2]);
void random_character(place* p, int id, int color[3], character c,
                      int position[2]);
void handle_request(message m, int id);
int delete_place(int position[2], int id, character c);
int get_columns();
int get_rows();
void swap(place* new_place, place* old_place, int new_pos[2], int old_pos[2]);
void draw_swap(message m, place new_place, int old_pos[2]);
void clear_position(int pos[2]);

#endif
