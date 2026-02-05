#include "hex.h"
#include <math.h>
#include <string.h>
#include <time.h>

// ==================== AlphaZero-Inspired AI ====================
// Cette implémentation simplifie AlphaZero en utilisant MCTS avec
// une fonction d'évaluation heuristique au lieu d'un réseau de neurones.
// 
// AlphaZero complet nécessiterait :
// - Un réseau de neurones profond pour la politique et la valeur
// - Apprentissage par renforcement via auto-jeu
// - Optimisation des poids du réseau
//
// Cette version utilise :
// - MCTS guidé par une heuristique de position
// - Évaluation basée sur les distances minimales
// - Sélection de coups stratégiques

#define ALPHAZERO_ITERATIONS 30000
#define ALPHAZERO_EXPLORATION 1.0

// Nœud de l'arbre AlphaZero (similaire à MCTS mais avec valeur heuristique)
typedef struct AlphaZeroNode {
    HexGame state;
    int move_row, move_col;
    char player;
    int visits;
    double value_sum;  // Somme des valeurs
    double prior;      // Probabilité a priori (remplace la politique du réseau)
    struct AlphaZeroNode *parent;
    struct AlphaZeroNode *children[MAX_SIZE * MAX_SIZE];
    int num_children;
    int untried_moves[MAX_SIZE * MAX_SIZE][2];
    int num_untried;
} AlphaZeroNode;

// Fonction heuristique de politique (remplace le réseau de neurones)
// Donne une probabilité a priori à chaque coup basée sur sa position stratégique
double compute_prior(HexGame *game, int row, int col, char player) {
    // Priorité aux coups centraux et aux coups qui prolongent les chemins
    double priority = 1.0;
    
    // Bonus pour les positions centrales
    int center = BOARD_SIZE / 2;
    int dist_from_center = abs(row - center) + abs(col - center);
    priority += (BOARD_SIZE - dist_from_center) * 0.1;
    
    // Bonus pour les coups adjacents aux pions existants
    int cases_adjacentes[6][2] = {{-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}};
    for (int k = 0; k < 6; k++) {
        int nr = row + cases_adjacentes[k][0];
        int nc = col + cases_adjacentes[k][1];
        if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
            if (game->grid[nr][nc] == player) {
                priority += 2.0; // Fort bonus pour connexion
            }
        }
    }
    
    return priority;
}

// Fonction de valeur heuristique (remplace le réseau de neurones)
// Estime la valeur de la position pour le joueur donné
double compute_value(HexGame *game, char player) {
    int score = eval(game);
    
    // Convertir le score en valeur normalisée [-1, 1]
    if (score == 1000) return (player == PLAYER1) ? 1.0 : -1.0;
    if (score == -1000) return (player == PLAYER1) ? -1.0 : 1.0;
    
    // Normalisation du score
    double normalized = (double)score / 100.0;
    if (normalized > 1.0) normalized = 1.0;
    if (normalized < -1.0) normalized = -1.0;
    
    return (player == PLAYER1) ? normalized : -normalized;
}

// Créer un nœud AlphaZero
AlphaZeroNode *alphazero_create_node(HexGame *state, int move_row, int move_col, 
                                      char player, double prior, AlphaZeroNode *parent) {
    AlphaZeroNode *node = (AlphaZeroNode *)malloc(sizeof(AlphaZeroNode));
    if (!node) {
        fprintf(stderr, "Erreur d'allocation mémoire AlphaZero\n");
        exit(1);
    }
    node->state = *state;
    node->move_row = move_row;
    node->move_col = move_col;
    node->player = player;
    node->visits = 0;
    node->value_sum = 0.0;
    node->prior = prior;
    node->parent = parent;
    node->num_children = 0;
    
    // Initialiser les coups non essayés
    node->num_untried = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->grid[i][j] == EMPTY) {
                node->untried_moves[node->num_untried][0] = i;
                node->untried_moves[node->num_untried][1] = j;
                node->num_untried++;
            }
        }
    }
    
    for (int i = 0; i < MAX_SIZE * MAX_SIZE; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

// Libérer l'arbre AlphaZero
void alphazero_free_tree(AlphaZeroNode *node) {
    if (node == NULL) return;
    for (int i = 0; i < node->num_children; i++) {
        alphazero_free_tree(node->children[i]);
    }
    free(node);
}

// Vérifier victoire
char alphazero_check_winner(HexGame *game) {
    int score = eval(game);
    if (score == 1000) return PLAYER1;
    if (score == -1000) return PLAYER2;
    return EMPTY;
}

// Calcul PUCT (Predictor + Upper Confidence Bound for Trees)
// Formule AlphaZero : Q(s,a) + c * P(s,a) * sqrt(N(s)) / (1 + N(s,a))
double puct_score(AlphaZeroNode *node, double exploration) {
    if (node->visits == 0) return 1e9;
    
    double q_value = node->value_sum / node->visits;
    double u_value = exploration * node->prior * sqrt(node->parent->visits) / (1.0 + node->visits);
    
    return q_value + u_value;
}

// Sélection avec PUCT
AlphaZeroNode *alphazero_select(AlphaZeroNode *node) {
    while (node->num_untried == 0 && node->num_children > 0) {
        double best_puct = -1e9;
        AlphaZeroNode *best_child = NULL;
        for (int i = 0; i < node->num_children; i++) {
            double val = puct_score(node->children[i], ALPHAZERO_EXPLORATION);
            if (val > best_puct) {
                best_puct = val;
                best_child = node->children[i];
            }
        }
        node = best_child;
    }
    return node;
}

// Expansion avec calcul des priors
AlphaZeroNode *alphazero_expand(AlphaZeroNode *node) {
    if (node->num_untried == 0) return node;
    
    // Calculer les priors pour tous les coups non essayés
    double priors[MAX_SIZE * MAX_SIZE];
    double total_prior = 0.0;
    
    char next_player = (node->player == PLAYER1) ? PLAYER2 : 
                       (node->player == PLAYER2) ? PLAYER1 : PLAYER1;
    
    for (int i = 0; i < node->num_untried; i++) {
        int row = node->untried_moves[i][0];
        int col = node->untried_moves[i][1];
        priors[i] = compute_prior(&node->state, row, col, next_player);
        total_prior += priors[i];
    }
    
    // Normaliser les priors
    if (total_prior > 0) {
        for (int i = 0; i < node->num_untried; i++) {
            priors[i] /= total_prior;
        }
    }
    
    // Choisir le coup avec la plus haute priorité
    int best_idx = 0;
    double best_prior = priors[0];
    for (int i = 1; i < node->num_untried; i++) {
        if (priors[i] > best_prior) {
            best_prior = priors[i];
            best_idx = i;
        }
    }
    
    int row = node->untried_moves[best_idx][0];
    int col = node->untried_moves[best_idx][1];
    
    // Retirer le coup de la liste
    node->untried_moves[best_idx][0] = node->untried_moves[node->num_untried - 1][0];
    node->untried_moves[best_idx][1] = node->untried_moves[node->num_untried - 1][1];
    node->num_untried--;
    
    // Créer le nouvel état
    HexGame new_state = node->state;
    new_state.grid[row][col] = next_player;
    
    // Créer le nœud enfant
    AlphaZeroNode *child = alphazero_create_node(&new_state, row, col, next_player, 
                                                  best_prior, node);
    node->children[node->num_children++] = child;
    
    return child;
}

// Évaluation (remplace la simulation par évaluation heuristique)
double alphazero_evaluate(HexGame *game, char player) {
    char winner = alphazero_check_winner(game);
    if (winner != EMPTY) {
        return (winner == player) ? 1.0 : -1.0;
    }
    return compute_value(game, player);
}

// Rétropropagation
void alphazero_backpropagate(AlphaZeroNode *node, double value, char root_player) {
    while (node != NULL) {
        node->visits++;
        // Alterner la perspective à chaque niveau
        node->value_sum += (node->player == root_player) ? value : -value;
        node = node->parent;
    }
}

// Fonction principale AlphaZero
void best_move_alphazero(HexGame *game, char player, int *bestRow, int *bestCol, int turn) {
    
    // Coups aléatoires en début de partie
    if (turn < RANDTOUR) {
        play_random_move(game, bestRow, bestCol);
        return;
    }
    
    // Créer la racine
    char prev_player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    AlphaZeroNode *root = alphazero_create_node(game, -1, -1, prev_player, 1.0, NULL);
    
    // Itérations AlphaZero
    for (int iter = 0; iter < ALPHAZERO_ITERATIONS; iter++) {
        // 1. Sélection
        AlphaZeroNode *node = alphazero_select(root);
        
        // Vérifier si terminé
        char winner = alphazero_check_winner(&node->state);
        
        if (winner == EMPTY && node->num_untried > 0) {
            // 2. Expansion
            node = alphazero_expand(node);
        }
        
        // 3. Évaluation (au lieu de simulation)
        double value = alphazero_evaluate(&node->state, player);
        
        // 4. Rétropropagation
        alphazero_backpropagate(node, value, player);
    }
    
    // Choisir le coup le plus visité
    int best_visits = -1;
    *bestRow = -1;
    *bestCol = -1;
    
    for (int i = 0; i < root->num_children; i++) {
        AlphaZeroNode *child = root->children[i];
        if (child->visits > best_visits) {
            best_visits = child->visits;
            *bestRow = child->move_row;
            *bestCol = child->move_col;
        }
    }
    
    // Fallback au cas où
    if (*bestRow == -1 || *bestCol == -1) {
        play_random_move(game, bestRow, bestCol);
    }
    
    // Libérer l'arbre
    alphazero_free_tree(root);
}
