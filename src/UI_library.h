#include <SDL2/SDL.h>

/*
 * description: get the board place based on the mouse position
 *
 * argument: mouse_x [int] - mouse x position on the window
 * argument: mouse_y [int] - mouse y position on the window
 * argument: board_x [int*] - board x position returned by reference
 * argument: board_y [int*] - board y position returned by reference
 */
void get_board_place(int mouse_x, int mouse_y, int *board_x, int *board_y);

/*
 * description: create a board window
 *
 * argument: dim_x [int] - width of the board
 * argument: dim_y [int] - height of the board
 * returns: int - always 0
 */
int create_board_window(int dim_x, int dim_y);

/*
 * description: close the board windows
 */
void close_board_windows();

/*
 * description: paint colored pacman
 *
 * argument: board_x [int] - board x position to paint pacman
 * argument: board_y [int] - board y position to paint pacman
 * argument: r [int] - color's red component
 * argument: g [int] - color's green component
 * argument: b [int] - color's blue component
 */
void paint_pacman(int board_x, int board_y, int r, int g, int b);

/*
 * description: paint colored superpowered pacman
 *
 * argument: board_x [int] - board x position to paint pacman
 * argument: board_y [int] - board y position to paint pacman
 * argument: r [int] - color's red component
 * argument: g [int] - color's green component
 * argument: b [int] - color's blue component
 */
void paint_powerpacman(int board_x, int board_y, int r, int g, int b);

/*
 * description: paint colored monster
 *
 * argument: board_x [int] - board x position to paint monster
 * argument: board_y [int] - board y position to paint monster
 * argument: r [int] - color's red component
 * argument: g [int] - color's green component
 * argument: b [int] - color's blue component
 */
void paint_monster(int board_x, int board_y, int r, int g, int b);

/*
 * description: paint a lemon
 *
 * argument: board_x [int] - board x position to paint lemon
 * argument: board_y [int] - board y position to paint lemon
 */
void paint_lemon(int board_x, int board_y);

/*
 * description: paint a cherry
 *
 * argument: board_x [int] - board x position to paint cherry
 * argument: board_y [int] - board y position to paint cherry
 */
void paint_cherry(int board_x, int board_y);

/*
 * description: paint a brick
 *
 * argument: board_x [int] - board x position to paint brick
 * argument: board_y [int] - board y position to paint brick
 */
void paint_brick(int board_x, int board_y);

/*
 * description: clear a place in the board
 *
 * argument: board_x [int] - board x position to clear
 * argument: board_y [int] - board y position to clear
 */
void clear_place(int board_x, int board_y);

/*
 * description: force the window to be rendered
 */
void render();
