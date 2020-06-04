/*****************************************************************************
 * File name: board.h
 *
 *  Author: Vasco Candeias (vascocandeias@tecnico.ulisboa.pt)
 *
 *  Release date: 01/06/2020
 *
 *  Description: Headers file for the board.c functions
 *
 ****************************************************************************/
#include <stdbool.h>
#include <stdlib.h>

#include "UI_library.h"
#include "message.h"

#ifndef BOARD_H
#define BOARD_H

/*
 * description: structure for each place in the board
 */
typedef struct place {
  character type;
  int id;
  int color[3];
  int kill_count;
} place;

/*
 * description: create a board structure, allocating the needed memory
 */
void create_board();

/*
 * description: initialize a board using a file or
 *
 * argument: filename [char*] - name of the txt file containing the initial
 *                              board configuration. if it is NULL, default
 *                              configuration is used
 * returns: int - number of free places available for the players
 */
int init_board(char* filename);

/*
 * description: delete and free the memory of the board structure
 */
void delete_board();

/*
 * description: determine if there is room for more players
 *
 * returns: true if there is still room for a new player and its fruits
 * returns: false otherwise
 */
bool has_room();

/*
 * description: generate a random position and place a character there
 *
 * argument: p [place*] - place to fill new coordinates
 * argument: position [int*] - coordinates of the position returned by reference
 */
void random_position(place* p, int position[2]);

/*
 * description: send the board structure to a player
 *
 * argument: fd [int] - file descriptor of the client
 * returns: 1 if successful
 * returns: -1 otherwise
 */
int send_board(int fd);

/*
 * description: get the pointer to the place with the given coordinates
 *
 * argument: position [int[2]] - coordinates of the desired place
 * returns: place* - the pointer to the place
 */
place* get_place(int position[2]);

/*
 * description: place a character in a random position on the board
 *
 * argument: p [place*] - place is returned by reference. cannot be NULL
 * argument: id [int] - character's id
 * argument: color [int[3]] - character's color
 * argument: c [character] - character's type
 * argument: position [int[2]] - position of the character returned by reference
 */
void random_character(place* p, int id, int color[3], character c,
                      int position[2]);

/*
 * description: handle the logic of a client's request
 *
 * argument: m [message] - player's move
 * argument: id [int] - player's id
 */
void handle_request(message m, int id);

/*
 * description: delete a place
 *
 * argument: position [int[2]] - coordinates for the place to be deletes
 * argument: id [int] - id of the character to be deleted
 * argument: type [character] - type of character to be deleted
 * returns: true if the deletition was successful
 * returns: false otherwise, that is, the player with this id and type has moved
 *          while calling or executing the function
 */
bool delete_place(int position[2], int id, character type);

/*
 * description: get the number of columns
 *
 * returns: int - number of columns
 */
int get_columns();

/*
 * description: get the number of rows
 *
 * returns: int - number of rows
 */
int get_rows();

/*
 * description: render the swap of two characters
 *
 * argument: m [message] - already contains the message corresponding to the
 *                         moving character, to be printed and sent
 * argument: new_place [place] - place now occupied by the moving character
 * argument: old_pos [int[2]] - final position of the moving character
 */
void draw_swap(message m, place new_place, int old_pos[2]);

/*
 * description: clears a position graphically, in the server and in the clients
 *
 * argument: pos [int[2]] - position to be cleared
 */
void clear_position(int pos[2]);

#endif
