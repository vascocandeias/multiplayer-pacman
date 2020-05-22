#include "players.h"

#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "UI_library.h"
#include "board.h"
#include "fruits.h"
#include "list.h"
#include "message.h"

typedef struct conditional {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  place* p;
  bool stop;
} conditional;

static List players = NULL;
static int n_players = 0;
static Uint32 event;

void init_players(Uint32 e) {
  pthread_t thread_id;
  event = e;
  pthread_create(&thread_id, NULL, thread_scoreboard, NULL);
}

void send_messages(message msg) {
  for (ListNode n = get_head(players); n; n = next(n)) {
    write(((player*)get_data(n))->id, &msg, sizeof(msg));
  }
}

void insert_player(player* p, int id) {
  p->id = id;
  p->pacman_score = 0;
  p->monster_score = 0;
  memset(p->monster, -1, sizeof(p->monster));
  memset(p->pacman, -1, sizeof(p->pacman));
  // printf("insert player %d\n", id);
  players = put(players, p);
  n_players++;
}

int* get_pacman(int id) {
  player* aux = (player*)get_item(players, id);
  return aux->pacman;
}

int* get_monster(int id) {
  player* aux = (player*)get_item(players, id);
  return aux->monster;
}

void set_monster(int id, int* position) {
  player* aux = (player*)get_item(players, id);
  memcpy(aux->monster, position, sizeof(int) * 2);
}

void set_pacman(int id, int* position) {
  player* aux = (player*)get_item(players, id);
  memcpy(aux->pacman, position, sizeof(int) * 2);
}

void init_player(int fd, player* p, place* pac, place* mon) {
  int color[3];
  message m;
  int pos[2];

  read(fd, &m, sizeof(m));

  for (int i = 0; i < 3; ++i) {
    if (m.color[i] > 255)
      m.color[i] = 255;
    else if (m.color[i] < 0)
      m.color[i] = 0;

    color[i] = m.color[i];
  }

  m.x = get_columns();
  m.y = get_rows();
  m.id = fd;

  write(fd, &m, sizeof(m));
  insert_player(p, fd);
  send_board(fd);

  memcpy(m.color, color, sizeof(color));
  m.old_x = -1;
  m.old_y = -1;

  random_character(pac, fd, color, PACMAN, pos);
  m.type = PACMAN;
  m.x = pos[0];
  m.y = pos[1];
  draw_character(m);

  random_character(mon, fd, color, MONSTER, pos);
  m.type = MONSTER;
  m.x = pos[0];
  m.y = pos[1];
  draw_character(m);
}

void draw_character(message m) {
  SDL_Event new_event;
  // create the data that will contain the new lemon position
  message* event_data = malloc(sizeof(message));
  if (!event_data) {
    perror("malloc");
    exit(-1);
  }
  *event_data = m;

  // clear the event data
  SDL_zero(new_event);
  // define event type
  new_event.type = event;
  // assign the event data
  new_event.user.data1 = event_data;
  // send the event
  SDL_PushEvent(&new_event);
}

void delete_player(int fd) {
  message m1, m2;
  m1.type = CLEAR;
  m2.type = CLEAR;
  m1.id = -1;
  m2.id = -1;
  int retries = 100;

  int* pos = NULL;
  do
    pos = get_monster(fd);
  while (!delete_place(pos, fd, MONSTER) && retries--);

  retries = 100;

  m1.old_x = pos[0];
  m1.old_y = pos[1];

  do
    pos = get_pacman(fd);
  while (!delete_place(pos, fd, PACMAN) && retries--);

  m2.old_x = pos[0];
  m2.old_y = pos[1];

  delete_node(players, fd);
  draw_character(m1);
  draw_character(m2);
  close(fd);
  if (--n_players == 1) {
    ((player*)get_data(get_head(players)))->monster_score = 0;
    ((player*)get_data(get_head(players)))->pacman_score = 0;
  }
  delete_two_fruits();
}

int get_n_players() { return n_players; }

void set_character(character c, int id, int pos[2]) {
  player* aux = (player*)get_item(players, id);
  if (c == PACMAN || c == POWER)
    memcpy(aux->pacman, pos, sizeof(int) * 2);
  else if (c == MONSTER)
    memcpy(aux->monster, pos, sizeof(int) * 2);
}

void delete_players() { delete_list(players); }

unsigned long get_time() {
  struct timespec t;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
  return t.tv_sec * 1000 + t.tv_nsec / 1000000;  // milliseconds
}

void* thread_user(void* arg) {
  message m;
  int err_rcv;
  int fd = *(int*)arg;

  // movement speed vars
  unsigned long t0 = get_time();
  unsigned long monster[] = {t0, t0};
  unsigned long pacman[] = {t0, t0};
  unsigned long* times[2] = {monster, pacman};
  int times_i[2] = {0, 0};
  unsigned long cur;
  int selected;

  // create structures
  player p;
  place pac, mon;
  init_player(fd, &p, &pac, &mon);

  // inactivity
  conditional inactive_m = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
                            &mon, false};
  conditional inactive_p = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
                            &pac, false};
  conditional inactives[2] = {inactive_p, inactive_m};

  pthread_t thread_ids[2];
  for (int i = 0; i < 2; ++i)
    pthread_create(&thread_ids[i], NULL, thread_inactive, &inactives[i]);

  while ((err_rcv = read(fd, &m, sizeof(m))) > 0) {
    if ((abs(m.x) + abs(m.y)) != 1) continue;
    switch (m.type) {
      case PACMAN:
      case POWER:
        selected = 0;
        break;
      case MONSTER:
        selected = 1;
        break;
      default:
        // not possible
        continue;
    }
    cur = get_time();
    if (times[selected][times_i[selected]] + 1000 > cur) continue;
    pthread_mutex_lock(&inactives[selected].mutex);
    pthread_cond_signal(&inactives[selected].cond);
    pthread_mutex_unlock(&inactives[selected].mutex);

    times[selected][times_i[selected]++] = cur;
    times_i[selected] %= 2;
    handle_request(m, fd);
  }

  printf("client %d disconnected\n", fd);
  for (int i = 0; i < 2; ++i) {
    pthread_mutex_lock(&inactives[i].mutex);
    inactives[i].stop = true;
    pthread_cond_signal(&inactives[i].cond);
    pthread_mutex_unlock(&inactives[i].mutex);
    pthread_join(thread_ids[i], NULL);
  }

  delete_player(fd);

  return NULL;
}

void* thread_inactive(void* arg) {
  conditional* input = (conditional*)arg;
  pthread_mutex_t* mutex = &input->mutex;
  pthread_cond_t* cond = &input->cond;
  place* p = input->p;
  message m = {-1, -1, -1, -1, p->id, p->type};
  memcpy(m.color, p->color, sizeof(m.color));
  bool* stop = &input->stop;
  struct timespec time_to_wait = {0};
  int pos[2];
  int retries;
  int* (*get_character)(int) = p->type == MONSTER ? get_monster : get_pacman;

  pthread_mutex_lock(mutex);

  while (1) {
    time_to_wait.tv_sec = time(0) + 30;
    if (pthread_cond_timedwait(cond, mutex, &time_to_wait)) {
      retries = 100;
      do
        memcpy(pos, get_character(p->id), sizeof(pos));
      while (!delete_place(pos, p->id, p->type) && retries--);
      m.old_x = pos[0];
      m.old_y = pos[1];
      random_position(p, pos);
      m.x = pos[0];
      m.y = pos[1];
      draw_character(m);
    }
    if (*stop) break;
  }

  pthread_mutex_unlock(mutex);
  pthread_mutex_destroy(mutex);
  pthread_cond_destroy(cond);
  return NULL;
}

void increase_score(int id, character type) {
  if (type % POWER == PACMAN)
    ((player*)get_item(players, id))->pacman_score++;
  else if (type == MONSTER)
    ((player*)get_item(players, id))->monster_score++;
}

void* thread_scoreboard(void* arg) {
  player* p;
  message m;
  while (1) {
    m.score = 0;
    if (n_players) printf("\nScore Board\n");
    for (ListNode n = get_head(players); n; n = next(n)) {
      p = (player*)get_data(n);
      m.x = p->id;
      m.y = p->pacman_score + p->monster_score;
      printf("Player %d: %d\n", m.x, m.y);
      send_messages(m);
      m.score = 1;
    }
    sleep(10);
  }
}
