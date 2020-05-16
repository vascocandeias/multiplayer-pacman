#include "board.h"

#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct message {
  int x, y, old_x, old_y;
  int id;
  character type;
  int color[3];
} message;

#endif
