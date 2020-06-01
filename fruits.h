#include <semaphore.h>

#include "message.h"

#ifndef FRUITS_H
#define FRUITS_H

typedef struct fruit fruit;

void init_fruits(int sz);
fruit* create_fruit(character type, int* id);
void* thread_fruit(void* arg);
void eat_fruit(int id);
void delete_fruits();
void delete_two_fruits();

#endif
