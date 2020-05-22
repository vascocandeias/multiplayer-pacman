#ifndef MESSAGE_H
#define MESSAGE_H

typedef enum character {
  PACMAN,
  MONSTER,
  BRICK,
  LEMON,
  CHERRY,
  CLEAR,
  POWER,
} character;

typedef struct message {
  int x, y, old_x, old_y;
  int id;
  character type;
  int color[3];
  int score;
} message;

#endif
