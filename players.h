#include "list.h"
#include "message.h"

#ifndef PLAYERS_H
#define PLAYERS_H

typedef struct player {
  int id;
  int pacman[2];
  int monster[2];
} player;

List insert_player(List players, player p, int id);
void send_messages(message msg, List players);

#endif
