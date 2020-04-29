#define PACMAN 1
#define MONSTER 2

typedef struct message {
  int x, y, old_x, old_y;
  int id;
  int character;
  int color[3];
} message;
