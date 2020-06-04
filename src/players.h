/*****************************************************************************
 * File name: players.h
 *
 *  Author: Vasco Candeias (vascocandeias@tecnico.ulisboa.pt)
 *
 *  Release date: 01/06/2020
 *
 *  Description: Headers file for the players.c functions
 *
 ****************************************************************************/
#include "UI_library.h"
#include "board.h"
#include "message.h"

#ifndef PLAYERS_H
#define PLAYERS_H

typedef struct player player;

/*
 * description: initialize player list
 *
 * argument: max_players [int] - max number of players
 * argument: event [Uint32] - event to be used by SDLPushEvent
 */
void init_players(int max_players, Uint32 event);

/*
 * description: insert a new player into the
 *
 * argument: fd [int] - file descriptor of the player's socket
 * returns: int - new player's id
 */
int insert_player(int fd);

/*
 * description: send message to every player
 *
 * argument: msg [message] - message to be sent
 */
void send_messages(message msg);

/*
 * description: get the position of the "id" player's monster
 *
 * argument: id [int] - player's id
 * returns: int* - position of the monster. might be undefined if the monster is
 *                 not placed or the player does not exist or NULL if id is
 *                 greater than n_players returns: NULL if id > max number of
 *                 players
 */
int* get_monster(int id);

/*
 * description: get the position of the "id" player's pacman
 *
 * argument: id [int] - player's id
 * returns: int* - position of the monster. might be undefined if the pacman is
 *                 not placed or the player does not exist or NULL if id is
 *                 greater than n_players returns: NULL if id > max number of
 *                 players
 */
int* get_pacman(int id);

/*
 * description: set a player's monster position
 *
 * argument: id [int] - player's id
 * argument: position [int[2]] - position where the monster was placed
 */
void set_monster(int id, int* position);

/*
 * description: set a player's pacman position
 *
 * argument: id [int] - player's id
 * argument: position [int[2]] - position where the pacman was placed
 */
void set_pacman(int id, int* position);

/*
 * description: calls one of the previous two functions, depending on the
 *              character
 *
 * argument: type [character] - either MONSTER, PACMAN or POWER
 * argument: id [int] - player's id
 * argument: position [int*] - position where the character was placed
 */
void set_character(character type, int id, int pos[2]);

/*
 * description: draw a character and send it to every player
 *
 * argument: msg [message] - contains the drawing information
 */
void draw_character(message msg);

/*
 * description: initialize a player
 *
 * argument: fd [int] - file descriptor of the player's socket
 * argument: pac [place*] - empty place structure passed by reference
 * argument: mon [place*] - empty place structure passed by reference
 * returns: int - player's id
 */
int init_player(int fd, place* pac, place* mon);

/*
 * description: delete a player
 *
 * argument: id [int] - player's id
 */
void delete_player(int id);

/*
 * description: delete every player and free the memory
 */
void delete_players();

/*
 * description: thread to handle user requests
 *
 * argument: arg [int*] - player's socket file descriptor
 * returns: void* - NULL
 */
void* thread_user(void* arg);

/*
 * description: thread to handle each player's character inactivity
 *
 * argument: args [conditional*] - structure with information regarding the
 *                                 conditional variable handling and the
 *                                 character's place pointer
 * returns: void* - NULL
 */
void* thread_inactive(void* args);

/*
 * description: return the number of players
 *
 * returns: int - number of players in the game
 */
int get_n_players();

/*
 * description: thread to periodically send the scoreboard to every player
 *
 * argument: arg [void*] - NULL
 * returns: void* - NULL
 */
void* thread_scoreboard(void* arg);

/*
 * description: increase a player's monster or pacman score
 *
 * argument: id [int] - player's id
 * argument: type [character] - character's type, either MONSTER, PACMAN or
 *                              POWER
 */
void increase_score(int id, character type);

#endif
