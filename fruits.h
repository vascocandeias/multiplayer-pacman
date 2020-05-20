#include <semaphore.h>

#include "message.h"

#ifndef FRUITS_H
#define FRUITS_H

void init_fruits(int sz);
int create_fruit(character type);
void *thread_fruit(void *arg);
void eat_fruit(int id);
void delete_fruits();
void delete_two_fruits();

#endif
