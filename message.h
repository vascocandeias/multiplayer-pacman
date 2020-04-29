// TODO: mudar para enum
#define INIT 1
#define PACMAN 2
#define MONSTER 3
#define POWER 4
#define BRICK 5
#define LEMON 6
#define CHERRY 7
#define CLEAR 8

typedef struct message {
  int x, y, old_x, old_y;
  int id;
  int character;
  int color[3];
} message;
