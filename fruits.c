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

typedef struct fruit {
  char name[100];
  sem_t *semaphore;
  character type;
  bool stop;
} fruit;

static fruit *fruits = NULL;
static int n_fruits;
static int size;
static pthread_mutex_t mutex;

int create_fruit(character type) {
  fruit f;
  int id;
  pthread_mutex_lock(&mutex);
  id = n_fruits++;
  pthread_mutex_unlock(&mutex);
  sprintf(f.name, "/fruits%d", id);

  f.type = type;
  f.stop = false;
  sem_unlink(f.name);
  f.semaphore = sem_open(f.name, O_CREAT | O_EXCL, S_IRWXU, 0);
  if (f.semaphore == SEM_FAILED) {
    perror("semaphore");
    exit(-1);
  }
  fruits[id] = f;
  return id;
}

void init_fruits(int sz) {
  size = sz;
  fruits = (fruit *)malloc(size * sizeof(fruit));
  if (!fruits) {
    perror("memory");
    exit(-1);
  }
  n_fruits = 0;
  pthread_mutex_init(&mutex, NULL);
}

void delete_fruits() {
  for (int i = 0; i < n_fruits; ++i) {
    fruits[i].stop = true;
    sem_post(fruits[i].semaphore);
    // TODO: join threads
  }
  free(fruits);
  pthread_mutex_destroy(&mutex);
}

void eat_fruit(int id) { sem_post(fruits[id].semaphore); }

void delete_two_fruits() {
  if (!n_fruits) return;
  pthread_mutex_lock(&mutex);
  fruits[--n_fruits].stop = true;
  sem_post(fruits[n_fruits].semaphore);
  fruits[--n_fruits].stop = true;
  sem_post(fruits[n_fruits].semaphore);
  pthread_mutex_unlock(&mutex);
}

void *thread_fruit(void *arg) {
  character type = *(character *)arg;
  int id = create_fruit(type);
  place p = {type, id, {0, 0, 0}};
  int pos[2];
  fruit *this = &fruits[id];
  message m = {-1, -1, -1, -1, id, type, {0, 0}};

  while (!this->stop) {
    random_position(&p, pos);
    m.x = pos[0];
    m.y = pos[1];
    m.old_x = -1;
    m.old_y = -1;
    draw_character(m);
    if (sem_wait(this->semaphore) == -1) {
      perror("semaphore wait");
      break;
    }
    if (this->stop) break;
    sleep(2);
  }
  if (delete_place(pos, id, type)) {
    m.old_x = pos[0];
    m.old_y = pos[1];
    m.type = CLEAR;
    draw_character(m);
  }
  pthread_mutex_lock(&mutex);
  sem_close(this->semaphore);
  this->semaphore = NULL;
  sem_unlink(this->name);
  pthread_mutex_unlock(&mutex);
  return NULL;
}
