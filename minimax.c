#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <omp.h>
#include "hex.h"

// Algorithme Minimax simple (sans élagage Alpha-Beta)
// Explore l'arbre des coups possibles jusqu'à une certaine profondeur.
int minimax(HexGame *game, int depth, bool isMax) {
    int score = eval(game);
    
    // Conditions d'arrêt : Victoire, Défaite ou Profondeur atteinte
    if (score == 1000 || score == -1000 || depth == 0) {
        return score;
    }

    if (isMax) { // Tour de PLAYER1 (Cherche à maximiser le score)
        int best = -INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER1; // Simule le coup
                    int val = minimax(game, depth - 1, false); // Appel récursif
                    game->grid[i][j] = EMPTY;   // Annule le coup (backtracking)
                    
                    if (val > best) best = val;
                }
            }
        }
        return best;
    } else { // Tour de PLAYER2 (Cherche à minimiser le score)
        int best = INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER2; // Simule le coup
                    int val = minimax(game, depth - 1, true); // Appel récursif
                    game->grid[i][j] = EMPTY;   // Annule le coup
                    
                    if (val < best) best = val;
                }
            }
        }
        return best;
    }
}

// Fonction principale pour lancer l'IA Minimax
// Utilise OpenMP pour paralléliser la recherche du premier niveau de coups.
void best_move_minimax(HexGame *game, char player, int *bestRow, int *bestCol, int turn) {
    // Si c'est le début de partie, on joue au hasard pour varier les ouvertures
    if (turn < RANDTOUR) {
        play_random_move(game, bestRow, bestCol);
        return;
    }

    int bestVal = (player == PLAYER1) ? -INF : INF;
    *bestRow = -1;
    *bestCol = -1;

    // Parallélisation de la boucle principale avec OpenMP
    // Chaque thread évalue un sous-ensemble des coups possibles
    #pragma omp parallel for num_threads(10) shared(game, bestVal, bestRow, bestCol)
    for (int idx = 0; idx < SIZE*SIZE; idx++) {
        int i = idx / SIZE;
        int j = idx % SIZE;
        if (game->grid[i][j] == EMPTY) {
            HexGame temp_game = *game; // Copie locale du jeu pour chaque thread
            temp_game.grid[i][j] = player;
            
            // Appel de Minimax pour évaluer ce coup
            int moveVal = minimax(&temp_game, PROFMINIMAX, (player == PLAYER2));

            // Section critique pour mettre à jour le meilleur coup global sans conflit
            #pragma omp critical
            {
                if (player == PLAYER1) { // Max
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        *bestRow = i;
                        *bestCol = j;
                    } else if (moveVal == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *bestRow = i;
                        *bestCol = j;
                    }
                } else { // Min
                    if (moveVal < bestVal) {
                        bestVal = moveVal;
                        *bestRow = i;
                        *bestCol = j;
                    } else if (moveVal == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *bestRow = i;
                        *bestCol = j;
                    }
                }
            }
        }
    }
}
