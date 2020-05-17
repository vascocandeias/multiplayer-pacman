#include "list.h"
#include "message.h"

#ifndef PLAYERS_H
#define PLAYERS_H

typedef struct player {
  int id;
  int pacman[2];
  int monster[2];
} player;

List insert_player(List players, player* p, int id);
void send_messages(message msg, List players);
int* get_monster(List players, int id);
int* get_pacman(List players, int id);
void set_monster(List players, int id, int* position);
void set_pacman(List players, int id, int* position);

#endif
