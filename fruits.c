#include "fruits.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board.h"
#include "message.h"
#include "players.h"

#define FRUIT_RESPAWN_TIME 2

struct fruit {
  char name[100];
  sem_t *semaphore;
  character type;
  bool stop;
};

static fruit **fruits = NULL;
static int n_fruits;
static int size;
static pthread_rwlock_t rwlock;

fruit *create_fruit(character type, int *id) {
  fruit *f = (fruit *)malloc(sizeof(fruit));

  f->type = type;
  f->stop = false;

  pthread_rwlock_wrlock(&rwlock);
  *id = n_fruits++;
  fruits[*id] = f;
  sprintf(f->name, "/fruits%d", *id);
  sem_unlink(f->name);
  f->semaphore = sem_open(f->name, O_CREAT | O_EXCL, S_IRWXU, 0);
  pthread_rwlock_unlock(&rwlock);
  if (f->semaphore == SEM_FAILED) {
    perror("semaphore");
    exit(-1);
  }
  return f;
}

void init_fruits(int sz) {
  size = sz;
  fruits = (fruit **)calloc(size, sizeof(fruit *));
  if (!fruits) {
    perror("memory");
    exit(-1);
  }
  n_fruits = 0;
  pthread_rwlock_init(&rwlock, NULL);
}

void delete_fruits() {
  for (int i = 0; i < n_fruits; ++i) {
    fruits[i]->stop = true;
    sem_post(fruits[i]->semaphore);
  }
  free(fruits);
}

void eat_fruit(int id) {
  pthread_rwlock_rdlock(&rwlock);
  if (fruits[id]) sem_post(fruits[id]->semaphore);
  pthread_rwlock_unlock(&rwlock);
}

void delete_two_fruits() {
  if (!n_fruits) return;
  pthread_rwlock_wrlock(&rwlock);
  fruits[--n_fruits]->stop = true;
  sem_post(fruits[n_fruits]->semaphore);
  fruits[n_fruits] = NULL;
  fruits[--n_fruits]->stop = true;
  sem_post(fruits[n_fruits]->semaphore);
  fruits[n_fruits] = NULL;
  pthread_rwlock_unlock(&rwlock);
}

void *thread_fruit(void *arg) {
  character type = *(character *)arg;
  int id;
  fruit *this = create_fruit(type, &id);
  place p = {type, id, {0, 0, 0}, -1};
  int pos[2];
  message m = {-1, -1, id, type, {0, 0}, -1};

  while (!this->stop) {
    random_position(&p, pos);
    m.x = pos[0];
    m.y = pos[1];
    draw_character(m);
    if (sem_wait(this->semaphore) == -1) {
      perror("semaphore wait");
      break;
    }
    if (this->stop) break;
    sleep(FRUIT_RESPAWN_TIME);
  }
  if (delete_place(pos, id, type)) clear_position(pos);
  sem_close(this->semaphore);
  this->semaphore = NULL;
  sem_unlink(this->name);
  free(this);
  printf("fruit deleted\n");
  return NULL;
}
