#include "UI_library.h"
#include "board.h"
#include "list.h"
#include "message.h"

#ifndef PLAYERS_H
#define PLAYERS_H

typedef struct player {
  int id;
  int pacman[2];
  int monster[2];
  int pacman_score;
  int monster_score;
} player;

void init_players(Uint32 e);
void insert_player(player* p, int id);
void send_messages(message msg);
int* get_monster(int id);
int* get_pacman(int id);
void set_monster(int id, int* position);
void set_pacman(int id, int* position);
void set_character(character c, int id, int pos[2]);
void draw_character(message m);
void init_player(int fd, player* p, place* pac, place* mon);
void delete_player(int fd);
void delete_players();
void* thread_user(void* arg);
void* thread_inactive(void* args);
int get_n_players();
void* thread_scoreboard(void* arg);
void increase_score(int id, character type);

#endif
