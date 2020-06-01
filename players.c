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
#include "message.h"

#define SCORE_BOARD_DELAY 60
#define INACTIVITY_TIMEOUT 30

typedef struct conditional {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  place* p;
  bool stop;
} conditional;

static player* players = NULL;
static int n_players = 0;
static int max = 0;
static Uint32 event;
static pthread_rwlock_t rwlock;

void init_players(int max_players, Uint32 e) {
  pthread_t thread_id;
  event = e;
  max = max_players;
  players = (player*)malloc(max * sizeof(player));
  if (!players) {
    perror("malloc");
    exit(-1);
  }
  for (int i = 0; i < max; ++i) players[i].id = -1;
  pthread_create(&thread_id, NULL, thread_scoreboard, NULL);
}

void send_messages(message msg) {
  int n = 0;
  for (int i = 0; i < max && n < n_players; ++i)
    if (players[i].id != -1) {
      if (write(players[i].id, &msg, sizeof(msg)) == -1) {
        perror("send messages");
        continue;
      }
      ++n;
    };
}

int insert_player(int id) {
  player p = {id, {-1, -1}, {-1, -1}, 0, 0};
  int n = 0, i = 0;
  // lock to secure multiple threads creating players concurrently
  pthread_rwlock_wrlock(&rwlock);
  for (; i < max && n < n_players; ++i)
    if (players[i].id == -1) break;
  players[i] = p;
  n_players++;
  pthread_rwlock_unlock(&rwlock);
  return i;
}

int* get_pacman(int id) { return players[id].pacman; }

int* get_monster(int id) { return players[id].monster; }

void set_monster(int id, int* position) {
  memcpy(players[id].monster, position, sizeof(players[id].monster));
}

void set_pacman(int id, int* position) {
  memcpy(players[id].pacman, position, sizeof(players[id].pacman));
}

int init_player(int fd, place* pac, place* mon) {
  int color[3];
  message m;
  int pos[2];

  if (read(fd, &m, sizeof(m)) <= 0) {
    perror("read in init");
    return -1;
  }

  for (int i = 0; i < 3; ++i) {
    if (m.color[i] > 255)
      m.color[i] = 255;
    else if (m.color[i] < 0)
      m.color[i] = 0;

    color[i] = m.color[i];
  }

  int id = insert_player(fd);
  m.x = get_columns();
  m.y = get_rows();
  m.id = id;

  if (write(fd, &m, sizeof(m)) <= 0 || send_board(fd) == -1) {
    perror("write in insertion\n");
    players[id].id = -1;
    n_players--;
    return -1;
  }

  memcpy(m.color, color, sizeof(color));

  random_character(pac, id, color, PACMAN, pos);
  m.type = PACMAN;
  m.x = pos[0];
  m.y = pos[1];
  draw_character(m);

  random_character(mon, id, color, MONSTER, pos);
  m.type = MONSTER;
  m.x = pos[0];
  m.y = pos[1];
  draw_character(m);
  return id;
}

void draw_character(message m) {
  SDL_Event new_event;
  m.score = -1;
  pthread_rwlock_rdlock(&rwlock);
  send_messages(m);
  pthread_rwlock_unlock(&rwlock);

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

void delete_player(int id) {
  int retries = 100;

  int* pos = NULL;
  do
    pos = get_monster(id);
  while (!delete_place(pos, id, MONSTER) && retries--);

  retries = 100;
  clear_position(pos);

  do
    pos = get_pacman(id);
  while (!delete_place(pos, id, PACMAN) && retries--);

  clear_position(pos);
  delete_two_fruits();
  pthread_rwlock_wrlock(&rwlock);
  int fd = players[id].id;
  players[id].id = -1;
  n_players--;
  close(fd);
  for (int i = 0; i < max && n_players == 1; ++i)
    if (players[i].id != -1) {
      players[i].pacman_score = 0;
      players[i].monster_score = 0;
      break;
    }
  pthread_rwlock_unlock(&rwlock);
}

int get_n_players() { return n_players; }

void set_character(character c, int id, int pos[2]) {
  // player* aux = (player*)get_item(players, id);
  player* aux = &players[id];
  if (c == PACMAN || c == POWER)
    memcpy(aux->pacman, pos, sizeof(int) * 2);
  else if (c == MONSTER)
    memcpy(aux->monster, pos, sizeof(int) * 2);
}

void delete_players() { free(players); }

unsigned long get_time() {
  struct timespec t;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
  return t.tv_sec * 1000 + t.tv_nsec / 1000000;  // milliseconds
}

void* thread_user(void* arg) {
  message m;
  int fd = *(int*)arg;

  // movement speed vars
  unsigned long t0 = get_time();
  unsigned long times[2][2] = {{t0, t0}, {t0, t0}};
  int times_i[2] = {0, 0};
  unsigned long cur;
  int selected;

  // create structures
  place pac, mon;
  int id = init_player(fd, &pac, &mon);

  if (id == -1) return NULL;

  // inactivity
  conditional inactive_m = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
                            &mon, false};
  conditional inactive_p = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
                            &pac, false};
  conditional inactives[2] = {inactive_p, inactive_m};

  pthread_t thread_ids[2];
  for (int i = 0; i < 2; ++i)
    pthread_create(&thread_ids[i], NULL, thread_inactive, &inactives[i]);

  while (read(fd, &m, sizeof(m)) > 0) {
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
    handle_request(m, id);
  }

  printf("player %d disconnected\n", id);
  for (int i = 0; i < 2; ++i) {
    pthread_mutex_lock(&inactives[i].mutex);
    inactives[i].stop = true;
    pthread_cond_signal(&inactives[i].cond);
    pthread_mutex_unlock(&inactives[i].mutex);
    pthread_join(thread_ids[i], NULL);
  }

  delete_player(id);
  printf("deleted: %d\n", n_players);

  return NULL;
}

void* thread_inactive(void* arg) {
  conditional* input = (conditional*)arg;
  pthread_mutex_t* mutex = &input->mutex;
  pthread_cond_t* cond = &input->cond;
  place* p = input->p;
  message m = {-1, -1, p->id, p->type};
  memcpy(m.color, p->color, sizeof(m.color));
  bool* stop = &input->stop;
  struct timespec time_to_wait = {0};
  int pos[2];
  int retries;
  int* (*get_character)(int) = p->type == MONSTER ? get_monster : get_pacman;

  pthread_mutex_lock(mutex);

  while (1) {
    time_to_wait.tv_sec = time(0) + INACTIVITY_TIMEOUT;
    if (pthread_cond_timedwait(cond, mutex, &time_to_wait)) {
      retries = 100;
      do
        memcpy(pos, get_character(p->id), sizeof(pos));
      while (!delete_place(pos, p->id, p->type) && retries--);
      clear_position(pos);
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
  // lock avoids conflicts with resetting
  pthread_rwlock_rdlock(&rwlock);
  if (type % POWER == PACMAN)
    players[id].pacman_score++;
  else if (type == MONSTER)
    players[id].monster_score++;
  pthread_rwlock_unlock(&rwlock);
}

void* thread_scoreboard(void* arg) {
  player p;
  message m;
  int n;

  while (1) {
    m.score = 0;
    n = 0;
    pthread_rwlock_rdlock(&rwlock);
    if (n_players) printf("\nScore Board\n");
    for (int i = 0; i < max && n < n_players; ++i)
      if (players[i].id != -1) {
        p = players[i];
        m.x = i;
        m.y = p.pacman_score + p.monster_score;
        printf("Player %d: %d\n", m.x, m.y);
        send_messages(m);
        m.score = 1;
        ++n;
      };
    pthread_rwlock_unlock(&rwlock);
    sleep(SCORE_BOARD_DELAY);
  }
}
