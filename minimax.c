#include "hex.h"

// Algorithme Minimax
int minimax(HexGame *game, int prof, bool isMax) {
    int score = eval(game);

    // Victoire, défaite ou limite de profondeur   
    if (score == 1000 || score == -1000 || prof == 0) {
        return score;
    }

    // Tour de Max (PLAYER1)
    if (isMax) {
        int best = -INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER1; // Simule le coup
                    int val = minimax(game, prof - 1, false); // Appel récursif
                    game->grid[i][j] = EMPTY;   // Annule le coup
                    
                    if (val > best) best = val;
                }
            }
        }
        return best;
    // Tour de Min (PLAYER2)
    } else {
        int best = INF;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->grid[i][j] == EMPTY) {
                    game->grid[i][j] = PLAYER2; // Simule le coup
                    int val = minimax(game, prof - 1, true); // Appel récursif
                    game->grid[i][j] = EMPTY;   // Annule le coup
                    
                    if (val < best) best = val;
                }
            }
        }
        return best;
    }
}

// Minimax + parallélisation avec OpenMP au premier niveau
void best_move_minimax(HexGame *game, char player, int *bestRow, int *bestCol, int turn) {

    if (turn < RANDTOUR) {
        play_random_move(game, bestRow, bestCol);
        return;
    }

    int bestVal = (player == PLAYER1) ? -INF : INF;
    *bestRow = -1;
    *bestCol = -1;

    #pragma omp parallel for num_threads(MAX_THREADS) shared(game, bestVal, bestRow, bestCol)
    for (int idx = 0; idx < SIZE*SIZE; idx++) {
        int i = idx / SIZE;
        int j = idx % SIZE;
        if (game->grid[i][j] == EMPTY) {
            HexGame temp_game = *game; // Copie locale pour le multi-threading
            temp_game.grid[i][j] = player;
            
            // Appel de Minimax pour évaluer ce coup
            int val = minimax(&temp_game, PROFMINIMAX, (player == PLAYER2));

            #pragma omp critical
            {
                if (player == PLAYER1) { // Max
                    if (val > bestVal) {
                        bestVal = val;
                        *bestRow = i;
                        *bestCol = j;
                    } else if (val == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *bestRow = i;
                        *bestCol = j;
                    }
                } else { // Min
                    if (val < bestVal) {
                        bestVal = val;
                        *bestRow = i;
                        *bestCol = j;
                    } else if (val == bestVal && (rand() % 2 == 0)) {
                        // Choix aléatoire en cas d'égalité
                        *bestRow = i;
                        *bestCol = j;
                    }
                }
            }
        }
    }
}
