#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hex.h"

//Cases adjacentes pour une grille hexagonale
int cases_adjacentes[6][2] = {{-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}};

// Initialise la grille avec des cases vides
void init_game(HexGame *game) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            game->grid[i][j] = EMPTY;
        }
    }
}

// Affiche board avec les couleurs en hexagonal
void print_board(HexGame *game) {
    printf("\n  ");
    for (int j = 0; j < SIZE; j++) printf("%c ", 'A' + j); // colonnes
    printf("\n");
    for (int i = 0; i < SIZE; i++) { // lignes
        for (int k = 0; k < i; k++) printf(" "); // décalage hexagonal
        printf("%2d", i + 1); // les num ligne
        for (int j = 0; j < SIZE; j++) {
            char c = game->grid[i][j];
            // Joueur 1 rouge et Joueur 2 bleu
            if (c == PLAYER1) printf(" \033[1;31m%c\033[0m", c);
            else if (c == PLAYER2) printf(" \033[1;34m%c\033[0m", c);
            else printf(" %c", c);
        }
        printf("\n");
    }
    printf("\n");
}

// Vérifie si le coup est dans la grille et case vide
bool valid_move(HexGame *game, int row, int col) {
    return (row >= 0 && row < SIZE && col >= 0 && col < SIZE && game->grid[row][col] == EMPTY);
}

// Algorithme BFS (Breadth-First Search) pour trouver la distance la plus courte
// pour qu'un joueur relie ses deux bords.
// Retourne la distance minimale ou INF si le chemin est bloqué.
int min_distance(HexGame *game, char player) {
    int dist[SIZE][SIZE];
    // Initialisation des distances à l'infini
    for(int i=0; i<SIZE; i++) for(int j=0; j<SIZE; j++) dist[i][j] = INF;

    int queue[SIZE * SIZE * 10]; // File pour le BFS
    int front = 0, rear = 0;

    // Initialisation de la file avec les cases du bord de départ
    if (player == PLAYER1) { // Haut vers Bas (Ligne 0)
        for (int j = 0; j < SIZE; j++) {
            if (game->grid[0][j] == player) {
                dist[0][j] = 0; // Déjà occupé par le joueur : coût 0
                queue[rear++] = 0 * SIZE + j;
            } else if (game->grid[0][j] == EMPTY) {
                dist[0][j] = 1; // Case vide : coût 1
                queue[rear++] = 0 * SIZE + j;
            }
        }
    } else { // Gauche vers Droite (Colonne 0)
        for (int i = 0; i < SIZE; i++) {
            if (game->grid[i][0] == player) {
                dist[i][0] = 0;
                queue[rear++] = i * SIZE + 0;
            } else if (game->grid[i][0] == EMPTY) {
                dist[i][0] = 1;
                queue[rear++] = i * SIZE + 0;
            }
        }
    }

    int min_dist = INF;

    // Boucle principale du BFS
    while (front < rear) {
        int curr = queue[front++];
        int r = curr / SIZE;
        int c = curr % SIZE;

        // Si on a atteint le bord opposé, on met à jour la distance minimale trouvée
        if ((player == PLAYER1 && r == SIZE - 1) || (player == PLAYER2 && c == SIZE - 1)) {
            if (dist[r][c] < min_dist) min_dist = dist[r][c];
        }

        // Exploration des voisins
        for (int k = 0; k < 6; k++) {
            int nr = r + cases_adjacentes[k][0];
            int nc = c + cases_adjacentes[k][1];

            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                char cell = game->grid[nr][nc];
                // Poids : 0 si case déjà au joueur, 1 si vide, INF si adversaire
                int weight = (cell == player) ? 0 : ((cell == EMPTY) ? 1 : INF);
                
                // Relaxation de l'arête
                if (weight != INF && dist[r][c] + weight < dist[nr][nc]) {
                    dist[nr][nc] = dist[r][c] + weight;
                    queue[rear++] = nr * SIZE + nc;
                }
            }
        }
    }
    return min_dist;
}

// Fonction d'évaluation heuristique du plateau
// Retourne un score positif si J1 a l'avantage, négatif si J2 a l'avantage.
int eval(HexGame *game) {
    int d1 = min_distance(game, PLAYER1);  // Distance pour Joueur 1 (Rouge)
    int d2 = min_distance(game, PLAYER2);  // Distance pour Joueur 2 (Bleu)

    if (d1 == 0) return 1000;      // Joueur 1 a gagné
    if (d2 == 0) return -1000;     // Joueur 2 a gagné
    if (d1 == INF) return -900;    // Joueur 1 bloqué, Joueur 2 gagne
    if (d2 == INF) return 900;     // Joueur 2 bloqué, Joueur 1 gagne
    return d2 - d1;                // Score normal : différence de distances
}

void play_random_move(HexGame *game, int *row, int *col) {
    int empty_cells[SIZE * SIZE][2];
    int count = 0;
    
    // Liste toutes les cases vides
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game->grid[i][j] == EMPTY) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }
    
    // Choisit une case au hasard
    if (count > 0) {
        int r = rand() % count;
        *row = empty_cells[r][0];
        *col = empty_cells[r][1];
    } else {
        *row = -1;
        *col = -1;
    }
}
