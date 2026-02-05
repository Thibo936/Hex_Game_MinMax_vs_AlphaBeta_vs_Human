#ifndef HEX_H
#define HEX_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

#define TYPE_HUMAN 1
#define TYPE_MINIMAX 2
#define TYPE_ALPHABETA 3
#define TYPE_MCTS 4
#define SIZE 6 // Taille du plateau 6x6
#define EMPTY '.'
#define PLAYER1 'X' // Joueur 1 Rouge Haut-Bas
#define PLAYER2 'O' // Joueur 2 Bleu Gauche-Droite
#define INF 10000
#define RANDTOUR 0 // Nombre de tours rand avant que l'IA joue
#define PROFMINIMAX 4 // Profondeur Minimax
#define PROFALPHABETA 4 // Profondeur Alpha-Bêta
#define MCTS_ITERATIONS 50000 // Nombre d'itérations MCTS
#define MCTS_EXPLORATION 1.414 // Constante d'exploration UCB1 (sqrt(2))
#define MAX_THREADS 10 // Nombre maximum de threads pour OpenMP

typedef struct {
    char grid[SIZE][SIZE];
} HexGame;

// hex_game.c
void init_game(HexGame *game);
void print_board(HexGame *game);
bool valid_move(HexGame *game, int row, int col);
int eval(HexGame *game);
void play_random_move(HexGame *game, int *row, int *col);

// minimax.c
void best_move_minimax(HexGame *game, char player, int *row, int *col, int turn);

// alphabeta.c
void best_move_alphabeta(HexGame *game, char player, int *row, int *col, int turn);

// mcts.c
void best_move_mcts(HexGame *game, char player, int *row, int *col, int turn);

#endif
