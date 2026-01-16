#include "hex.h"
 
int cases_adjacentes[6][2] = {{-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}}; //(row, col)

void init_game(HexGame *game) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            game->grid[i][j] = EMPTY;
        }
    }
}

void print_board(HexGame *game) {
    printf("\n  ");
    for (int j = 0; j < SIZE; j++) printf("%c ", 'A' + j); // colonnes
    printf("\n");
    for (int i = 0; i < SIZE; i++) { // lignes
        for (int k = 0; k < i; k++) {
            printf(" "); // décalage
        }
        printf("%d", i + 1); // Numéro de ligne
        for (int j = 0; j < SIZE; j++) {
            char c = game->grid[i][j];
            // Joueur 1 rouge et Joueur 2 bleu
            if (c == PLAYER1) printf(" \033[1;31m%c\033[0m", c);
            else if (c == PLAYER2) printf(" \033[1;34m%c\033[0m", c);
            else printf(" %c", c); // vide
        }
        printf("\n");
    }
    printf("\n");
}
// Vérifie si la case est valide et vide
bool valid_move(HexGame *game, int row, int col) {
    return (row >= 0 && row < SIZE && col >= 0 && col < SIZE && game->grid[row][col] == EMPTY);
}

// Algorithme 0-1 BFS (Deque) pour les poids 0 et 1.
// Cherche la distance minimale qu'un joueur doit parcourir pour gagner.
int min_distance(HexGame *game, char player) {
    int dist[SIZE][SIZE];
    for(int i=0; i<SIZE; i++) 
        for(int j=0; j<SIZE; j++) 
            dist[i][j] = INF;

    // Deque pour 0-1 BFS (on utilise un tableau circulaire)
    int deque[SIZE * SIZE * 2];
    int front = SIZE * SIZE; // Début au milieu pour pouvoir ajouter devant
    int rear = SIZE * SIZE;

    // Initialisation de la deque avec le bord de départ
    if (player == PLAYER1) { // Joueur X doit relier ligne 0 à ligne SIZE-1
        for (int j = 0; j < SIZE; j++) {
            if (game->grid[0][j] == player) {
                dist[0][j] = 0; // Case déjà occupée par le joueur coût 0
                deque[--front] = 0 * SIZE + j; // Poids 0 -> devant
            } else if (game->grid[0][j] == EMPTY) {
                dist[0][j] = 1; // Case vide coût 1
                deque[rear++] = 0 * SIZE + j; // Poids 1 -> derrière
            }
        }
    } else { // Joueur O doit relier colonne 0 à colonne SIZE-1
        for (int i = 0; i < SIZE; i++) {
            if (game->grid[i][0] == player) {
                dist[i][0] = 0;
                deque[--front] = i * SIZE + 0; // Poids 0 -> devant
            } else if (game->grid[i][0] == EMPTY) {
                dist[i][0] = 1;
                deque[rear++] = i * SIZE + 0; // Poids 1 -> derrière
            }
        }
    }

    // Boucle 0-1 BFS
    while (front < rear) {
        int curr = deque[front++];
        int r = curr / SIZE;
        int c = curr % SIZE;

        // Vérification des 6 voisins
        for (int k = 0; k < 6; k++) {
            int nr = r + cases_adjacentes[k][0];
            int nc = c + cases_adjacentes[k][1];

            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                char cell = game->grid[nr][nc];
                // 0 si occupé par soi, 1 si vide, INF si bloqué par l'autre
                int weight = (cell == player) ? 0 : ((cell == EMPTY) ? 1 : INF);
                
                if (weight != INF && dist[r][c] + weight < dist[nr][nc]) {
                    dist[nr][nc] = dist[r][c] + weight;
                    if (weight == 0) {
                        deque[--front] = nr * SIZE + nc; // Poids 0 -> devant
                    } else {
                        deque[rear++] = nr * SIZE + nc;  // Poids 1 -> derrière
                    }
                }
            }
        }
    }

    // Trouver la distance minimale vers le bord de victoire
    int min_dist = INF;
    if (player == PLAYER1) { // Joueur X doit atteindre ligne SIZE-1
        for (int j = 0; j < SIZE; j++) {
            if (dist[SIZE-1][j] < min_dist) min_dist = dist[SIZE-1][j];
        }
    } else { // Joueur O doit atteindre colonne SIZE-1
        for (int i = 0; i < SIZE; i++) {
            if (dist[i][SIZE-1] < min_dist) min_dist = dist[i][SIZE-1];
        }
    }
    
    return min_dist;
}

int eval(HexGame *game) {
    int d1 = min_distance(game, PLAYER1); 
    int d2 = min_distance(game, PLAYER2); 

    if (d1 == 0) return 1000;   // Joueur 1 a gagné
    if (d2 == 0) return -1000;  // Joueur 2 a gagné
    //if (d1 == INF) return -900; // Joueur 1 bloqué
    //if (d2 == INF) return 900;  // Joueur 2 bloqué
    return d2 - d1;
}

void play_random_move(HexGame *game, int *row, int *col) {
    int empty_cells[SIZE * SIZE][2];
    int count = 0;
    
    // Recherche de toutes les cases vides
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game->grid[i][j] == EMPTY) {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }
    
    if (count > 0) {
        int r = rand() % count;
        *row = empty_cells[r][0];
        *col = empty_cells[r][1];
    } else {
        *row = -1;
        *col = -1;
    }
}
