typedef enum character {
  CLEAR,
  PACMAN,
  MONSTER,
  POWER,
  BRICK,
  LEMON,
  CHERRY,
} character;

typedef struct message {
  int x, y, old_x, old_y;
  int id;
  character type;
  int color[3];
} message;
