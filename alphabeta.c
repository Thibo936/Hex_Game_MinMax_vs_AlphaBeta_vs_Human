#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <omp.h>
#include "hex.h"

// Algorithme Alpha-Bêta séquentiel pur
// Optimisation de Minimax qui coupe les branches inutiles de l'arbre.
int alphabeta(HexGame *game, int depth, int alpha, int beta, bool isMax) {
    int score = eval(game);
    
    // Conditions d'arrêt
    if (score == 1000 || score == -1000 || depth == 0) {
        return score;
    }

    if (isMax) { // Tour de Max (PLAYER1)
        int best = -INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER1;
                    int val = alphabeta(game, depth - 1, alpha, beta, false);
                    game->grid[i][j] = EMPTY;
                    
                    if (val > best) best = val;
                    if (best > alpha) alpha = best; // Mise à jour de la borne inférieure
                    if (beta <= alpha) return best; // Coupure Beta : l'adversaire ne laissera pas jouer ce coup
                }
            }
        }
        return best;
    } else { // Tour de Min (PLAYER2)
        int best = INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER2;
                    int val = alphabeta(game, depth - 1, alpha, beta, true);
                    game->grid[i][j] = EMPTY;
                    
                    if (val < best) best = val;
                    if (best < beta) beta = best; // Mise à jour de la borne supérieure
                    if (beta <= alpha) return best; // Coupure Alpha
                }
            }
        }
        return best;
    }
}

// Fonction principale pour lancer l'IA Alpha-Beta
// Parallélisée avec OpenMP au premier niveau
void best_move_alphabeta(HexGame *game, char player, int *row, int *col, int turn) {

    if (turn < RANDTOUR) {
        play_random_move(game, row, col);
        return;
    }

    int bestVal = (player == PLAYER1) ? -INF : INF;
    *row = -1; *col = -1;

//    for (int i = 0; i < SIZE; i++) {
//        for (int j = 0; j < SIZE; j++) {
//            if (game->grid[i][j] == EMPTY) {
//                game->grid[i][j] = player;
//                int val = alphabeta(game, PROFALPHABETA, -INF, INF, (player == PLAYER2));
//                game->grid[i][j] = EMPTY;
//
    // Parallélisation OpenMP
    #pragma omp parallel for num_threads(10) shared(game, bestVal, row, col)
    for (int idx = 0; idx < SIZE*SIZE; idx++) {
        int i = idx / SIZE;
        int j = idx % SIZE;

        if (game->grid[i][j] == EMPTY) {
            HexGame temp_game = *game; // Copie locale
            temp_game.grid[i][j] = player;
            
            // Appel récursif avec fenêtre alpha-beta initiale ouverte [-INF, INF]
            int val = alphabeta(&temp_game, PROFALPHABETA, -INF, INF, (player == PLAYER2));

            #pragma omp critical
            {   
                if (player == PLAYER1) {
                    if (val > bestVal) {
                        bestVal = val;
                        *row = i; *col = j;
                    }
                } else {
                    if (val < bestVal) {
                        bestVal = val;
                        *row = i; *col = j;
                    }
                }
            }
        }
    }
}
