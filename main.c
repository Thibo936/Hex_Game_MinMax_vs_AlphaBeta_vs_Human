#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include "hex.h"

#define TYPE_HUMAN 1
#define TYPE_MINIMAX 2
#define TYPE_ALPHABETA 3

// Jeux du joueur
void player_move(HexGame *game, char player, int *row, int *col) {
    char col_char;
    int r;
    char input[10];
    
    while (true) {
        printf("Entrez votre coup (ex: A1) : ");
        if (fgets(input, sizeof(input), stdin) == NULL) exit(0);
        
        // Lecture de l'entrée utilisateur (Lettre + Chiffre)
        if (sscanf(input, " %c%d", &col_char, &r) == 2) {
            // Conversion minuscule -> majuscule
            if (col_char >= 'a' && col_char <= 'z') col_char -= 32;
            int c = col_char - 'A'; // Conversion Lettre -> Index colonne (0-5)
            r = r - 1;              // Conversion Chiffre -> Index ligne (0-5)
            
            if (valid_move(game, r, c)) {
                *row = r;
                *col = c;
                break;
            } else {
                printf("Coup invalide.\n");
            }
        } else {
            printf("Format invalide.\n");
        }
    }
}

int main() {
    srand(time(NULL));
    HexGame game;
    init_game(&game);
    
    int type1, type2;

    printf("------------- JEU HEX -------------\n");
    printf("Pramètre Joueur 1 \033[31mX\033[0m - Haut/Bas) :\n");
    printf("1. Humain\n2. Minimax\n3. Alpha-Beta\nChoix : ");
    scanf("%d", &type1);
    
    printf("Pramètre Joueur 2 \033[34mO\033[0m - Gauche/Droite) :\n");
    printf("1. Humain\n2. Minimax\n3. Alpha-Beta\nChoix : ");
    scanf("%d", &type2);
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    int turn = 0;
    bool game_over = false;
    
    print_board(&game);

    while (!game_over) {
        char current_player = (turn % 2 == 0) ? PLAYER1 : PLAYER2;
        int current_type = (turn % 2 == 0) ? type1 : type2;
        
        printf("\n------------- Tour %d : du Joueur %c (%s) -------------\n", turn + 1, current_player, 
               (current_type == TYPE_HUMAN) ? "Humain" : "IA");

        int row = -1, col = -1;

        if (current_type == TYPE_HUMAN) {
            player_move(&game, current_player, &row, &col);
        } else if (current_type == TYPE_MINIMAX) {
            printf("Attente de Minimax \n");
            best_move_minimax(&game, current_player, &row, &col, turn);
        } else if (current_type == TYPE_ALPHABETA) {
            printf("Attente de Alpha-Beta \n");
            best_move_alphabeta(&game, current_player, &row, &col, turn);
        }

        if (row != -1 && col != -1) {
            game.grid[row][col] = current_player;
            if (current_type != TYPE_HUMAN) {
                printf("IA joue en %c%d\n", 'A' + col, row + 1);
            }
            print_board(&game);

            // Vérification de victoire via eval (si score +/- 1000)
            int score = eval(&game);
            if (score == 1000) {
                printf("\nVICTOIRE JOUEUR 1 (Rouge) !\n");
                game_over = true;
            } else if (score == -1000) {
                printf("\nVICTOIRE JOUEUR 2 (Bleu) !\n");
                game_over = true;
            }
        } else {
            printf("Erreur : Aucun coup trouvé.\n");
            break;
        }
        
        turn++;
    }

    return 0;
}
