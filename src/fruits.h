/*****************************************************************************
 * File name: fruits.h
 *
 *  Author: Vasco Candeias (vascocandeias@tecnico.ulisboa.pt)
 *
 *  Release date: 01/06/2020
 *
 *  Description: Headers file for the fruits.c functions
 *
 ****************************************************************************/
#include <semaphore.h>

#include "message.h"

#ifndef FRUITS_H
#define FRUITS_H

/*
 * description: structure to save each fruit information
 */
typedef struct fruit fruit;

/*
 * description: initialize the array of fruits
 *
 * argument: size [int] - maximum number of fruits
 */
void init_fruits(int size);

/*
 * description: create a fruit
 *
 * argument: type [character] - either LEMON or CHERRY
 * argument: id [int*] - fruit id, passed by reference
 * returns: fruit* - newly created fruit
 */
fruit* create_fruit(character type, int* id);

/*
 * description: thread responsible to manage a fruit. waits for fruit to be
 * 				eaten, then waits for two seconds, renders the
 * 				fruit again and starts over
 *
 * argument: arg [character*] - type of the fruit
 * returns: void* - NULL
 */
void* thread_fruit(void* arg);

/*
 * description: to call when a character eats a fruit. tells fruit the thread to
 * 	            sleep
 *
 * argument: id [int] - id of the eaten fruit
 */
void eat_fruit(int id);

/*
 * description: close and deletes every fruit semaphore
 */
void delete_fruits();

/*
 * description: delete two fruits if there are any
 */
void delete_two_fruits();

#endif
