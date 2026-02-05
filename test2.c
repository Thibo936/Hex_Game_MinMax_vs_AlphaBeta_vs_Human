#include <time.h>
#include <string.h>
#include "hex.h"

// Types de joueurs pour le test
#define TEST_RANDOM 0
#define TEST_ALPHABETA 2

// Simule un joueur humain qui joue aléatoirement
void play_random(HexGame *game, int *row, int *col) {
    play_random_move(game, row, col);
}

// Joue une partie complète et retourne le gagnant (1 ou 2)
int play_game(int type1, int type2, int *nb_tours) {
    HexGame game;
    init_game(&game);
    
    int turn = 0;
    
    while (1) {
        char current_player = (turn % 2 == 0) ? PLAYER1 : PLAYER2;
        int current_type = (turn % 2 == 0) ? type1 : type2;
        
        int row = -1, col = -1;
        
        if (current_type == TEST_RANDOM) {
            play_random(&game, &row, &col);
        } else if (current_type == TEST_ALPHABETA) {
            best_move_alphabeta(&game, current_player, &row, &col, turn);
        }
        
        if (row == -1 || col == -1) {
            *nb_tours = turn;
            return 0;
        }
        
        game.grid[row][col] = current_player;
        
        int score = eval(&game);
        if (score == 1000) {
            *nb_tours = turn + 1;
            return 1; // Joueur 1 gagne
        } else if (score == -1000) {
            *nb_tours = turn + 1;
            return 2; // Joueur 2 gagne
        }
        
        turn++;
        if (turn > SIZE * SIZE) {
            *nb_tours = turn;
            return 0;
        }
    }
}

int main() {
    srand(time(NULL));
    
    printf("============================================================\n");
    printf("        TEST : RANDOM (J1) vs ALPHA-BETA (J2)\n");
    printf("        Jusqu'à la victoire du Joueur Aléatoire\n");
    printf("============================================================\n\n");
    
    int nb_parties = 0;
    int winner = 0;
    int nb_tours;
    
    while (winner != 1) {
        nb_parties++;
        winner = play_game(TEST_RANDOM, TEST_ALPHABETA, &nb_tours);
        
        printf("\rPartie %d terminée... %s", 
               nb_parties, (winner == 1) ? "VICTOIRE DU RANDOM !" : "Le Random a perdu.");
        fflush(stdout);
        
        if (winner == 1) {
            printf("\n\n=> VICTOIRE DU RANDOM (Rouge) après %d parties !\n", nb_parties);
            printf("   La partie gagnante a duré %d tours.\n", nb_tours);
            printf("============================================================\n");
        }
    }
    
    return 0;
}
