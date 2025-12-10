#ifndef HEX_H
#define HEX_H

#include <stdbool.h>

#define TYPE_HUMAN 1
#define TYPE_MINIMAX 2
#define TYPE_ALPHABETA 3
#define SIZE 6              
#define EMPTY '.'
#define PLAYER1 'X' // Joueur 1 : Rouge Haut-Bas
#define PLAYER2 'O' // Joueur 2 : Bleu Gauche-Droite
#define INF 10000
#define RANDTOUR 4 // Nombre de tours randome avant que l'IA joue
#define PROFMINIMAX 4
#define PROFALPHABETA 4

typedef struct {
    char grid[SIZE][SIZE];
} HexGame;

// dans hex_game.c
void init_game(HexGame *game);
void print_board(HexGame *game);
bool valid_move(HexGame *game, int row, int col);
int eval(HexGame *game);
void play_random_move(HexGame *game, int *row, int *col);

// dans minimax.c
void best_move_minimax(HexGame *game, char player, int *row, int *col, int turn);

// dans alphabeta.c
void best_move_alphabeta(HexGame *game, char player, int *row, int *col, int turn);

#endif
