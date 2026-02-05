#include "hex.h"

// Algorithme Alpha-Bêta
int alphabeta(HexGame *game, int prof, int alpha, int beta, bool isMax) {
    int score = eval(game);

    // Victoire, défaite ou limite de profondeur
    if (score == 1000 || score == -1000 || prof == 0) {
        return score;
    }
    
    // Tour de Max (PLAYER1)
    if (isMax) { 
        int best = -INF;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER1; // Simule le coup
                    int val = alphabeta(game, prof - 1, alpha, beta, false); // Appel récursif
                    game->grid[i][j] = EMPTY; // Annule le coup
                    
                    if (val > best) best = val;
                    if (best > alpha) alpha = best; // Maj de la borne inférieure
                    if (beta <= alpha) return best; // L'autre player va bloquer ce coup
                }
            }
        }
        return best;
    // Tour de Min (PLAYER2)
    } else { 
        int best = INF;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER2; // Simule le coup
                    int val = alphabeta(game, prof - 1, alpha, beta, true); // Appel récursif
                    game->grid[i][j] = EMPTY; // Annule le coup
                    
                    if (val < best) best = val;
                    if (best < beta) beta = best; // Maj de la borne supérieure
                    if (beta <= alpha) return best; // L'autre player va bloquer ce coup
                }
            }
        }
        return best;
    }
}

// Alpha-Beta + parallélisation avec OpenMP au premier niveau
void best_move_alphabeta(HexGame *game, char player, int *Bestrow, int *Bestcol, int turn) {

    // Coups aléatoires en début de partie
    if (turn < RANDTOUR) {
        play_random_move(game, Bestrow, Bestcol);
        return;
    }

    int bestVal = (player == PLAYER1) ? -INF : INF;
    *Bestrow = -1; 
    *Bestcol = -1;

    // Parallélisation OpenMP au niveau 0 de l'arbre
    #pragma omp parallel for num_threads(MAX_THREADS) shared(game, bestVal, Bestrow, Bestcol)
    for (int idx = 0; idx < BOARD_SIZE*BOARD_SIZE; idx++) {
        int i = idx / BOARD_SIZE;
        int j = idx % BOARD_SIZE;

        if (game->grid[i][j] == EMPTY) {
            HexGame temp_game = *game; // Copie locale pour le multi-threading
            temp_game.grid[i][j] = player;
            
            // Appel de l'Alpha-Bêta pour évaluer ce coup
            int val = alphabeta(&temp_game, PROFALPHABETA, -INF, INF, (player == PLAYER2));

            #pragma omp critical
            {   
                if (player == PLAYER1) { // Max
                    if (val > bestVal) {
                        bestVal = val;
                        *Bestrow = i; 
                        *Bestcol = j;
                    } else if (val == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *Bestrow = i; 
                        *Bestcol = j;
                    }
                } else { // Min
                    if (val < bestVal) {
                        bestVal = val;
                        *Bestrow = i; 
                        *Bestcol = j;
                    } else if (val == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *Bestrow = i; 
                        *Bestcol = j;
                    }
                }
            }
        }
    }
}
