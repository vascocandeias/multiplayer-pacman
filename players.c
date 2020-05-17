#include "players.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list.h"
#include "message.h"

void send_messages(message msg, List sockets) {
  for (ListNode n = get_head(sockets); n; n = next(n)) {
    printf("send to %d\n", ((player*)get_data(n))->id);
    write(((player*)get_data(n))->id, &msg, sizeof(msg));
  }
}

List insert_player(List players, player* p, int id) {
  p->id = id;
  memset(p->monster, -1, sizeof(p->monster));
  memset(p->pacman, -1, sizeof(p->pacman));

  return put(players, p);
}

int* get_pacman(List players, int id) {
  player* aux = (player*)get_item(players, id);
  return aux->pacman;
}

int* get_monster(List players, int id) {
  player* aux = (player*)get_item(players, id);
  printf("monster at %d %d\n", aux->monster[0], aux->monster[1]);
  return aux->monster;
}

void set_monster(List players, int id, int* position) {
  printf("set monster %d\n", id);
  player* aux = (player*)get_item(players, id);
  memcpy(aux->monster, position, sizeof(int) * 2);
  printf("monster placed at %d %d -- %d %d\n", aux->monster[0], aux->monster[1],
         position[0], position[1]);
}

void set_pacman(List players, int id, int* position) {
  player* aux = (player*)get_item(players, id);
  memcpy(aux->pacman, position, sizeof(int) * 2);
}
