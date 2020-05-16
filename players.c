#include "players.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list.h"
#include "message.h"

void send_messages(message msg, List sockets) {
  for (ListNode n = get_head(sockets); n; n = next(n)) {
    printf("%d\n", ((player*)get_data(n))->id);
    write(((player*)get_data(n))->id, &msg, sizeof(msg));
  }
}

List insert_player(List players, player p, int id) {
  p.id = id;
  memset(p.monster, -1, sizeof(p.monster));
  memset(p.pacman, -1, sizeof(p.pacman));

  return put(players, &p);
}