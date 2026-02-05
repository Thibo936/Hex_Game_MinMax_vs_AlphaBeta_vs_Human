#include "hex.h"
#include <math.h>
#include <string.h>
#include <time.h>

// ==================== MCTS (Monte Carlo Tree Search) ====================
// L'algorithme MCTS explore l'arbre de jeu en 4 étapes :
// 1. Sélection   : Descend dans l'arbre avec UCB1 jusqu'à un nœud non exploré
// 2. Expansion   : Ajoute un nouveau nœud enfant
// 3. Simulation  : Joue une partie aléatoire (rollout) jusqu'à la fin
// 4. Rétropropagation : Remonte le résultat pour mettre à jour les statistiques

// Nœud de l'arbre MCTS
typedef struct MCTSNode {
    HexGame state;              // État du plateau à ce nœud
    int move_row, move_col;     // Coup qui a mené à ce nœud
    char player;                // Joueur qui a joué ce coup
    int visits;                 // Nombre de visites
    double wins;                // Nombre de victoires cumulées
    struct MCTSNode *parent;    // Nœud parent
    struct MCTSNode *children[MAX_SIZE * MAX_SIZE]; // Enfants possibles
    int num_children;           // Nombre d'enfants créés
    int untried_moves[MAX_SIZE * MAX_SIZE][2]; // Coups non encore essayés
    int num_untried;            // Nombre de coups non essayés
} MCTSNode;

// Crée un nouveau nœud MCTS
MCTSNode *mcts_create_node(HexGame *state, int move_row, int move_col, char player, MCTSNode *parent) {
    MCTSNode *node = (MCTSNode *)malloc(sizeof(MCTSNode));
    if (!node) {
        fprintf(stderr, "Erreur d'allocation mémoire MCTS\n");
        exit(1);
    }
    node->state = *state;
    node->move_row = move_row;
    node->move_col = move_col;
    node->player = player;
    node->visits = 0;
    node->wins = 0.0;
    node->parent = parent;
    node->num_children = 0;

    // Initialiser les coups non essayés (cases vides)
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

// Libère récursivement l'arbre MCTS
void mcts_free_tree(MCTSNode *node) {
    if (node == NULL) return;
    for (int i = 0; i < node->num_children; i++) {
        mcts_free_tree(node->children[i]);
    }
    free(node);
}

// Vérifie si la partie est terminée (un joueur a gagné)
// Retourne PLAYER1, PLAYER2 ou EMPTY (pas fini)
char mcts_check_winner(HexGame *game) {
    int score = eval(game);
    if (score == 1000) return PLAYER1;
    if (score == -1000) return PLAYER2;
    return EMPTY;
}

// Calcul de la valeur UCB1 (Upper Confidence Bound)
double ucb1(MCTSNode *node, double exploration) {
    if (node->visits == 0) return 1e9; // Priorité infinie pour les nœuds non visités
    return (node->wins / node->visits) + exploration * sqrt(log(node->parent->visits) / node->visits);
}

// Phase 1 : Sélection — descend dans l'arbre via UCB1
MCTSNode *mcts_select(MCTSNode *node) {
    while (node->num_untried == 0 && node->num_children > 0) {
        // Sélectionner l'enfant avec la meilleure valeur UCB1
        double best_ucb = -1e9;
        MCTSNode *best_child = NULL;
        for (int i = 0; i < node->num_children; i++) {
            double val = ucb1(node->children[i], MCTS_EXPLORATION);
            if (val > best_ucb) {
                best_ucb = val;
                best_child = node->children[i];
            }
        }
        node = best_child;
    }
    return node;
}

// Phase 2 : Expansion — ajoute un enfant au nœud
MCTSNode *mcts_expand(MCTSNode *node) {
    if (node->num_untried == 0) return node;

    // Choisir un coup non essayé au hasard
    int idx = rand() % node->num_untried;
    int row = node->untried_moves[idx][0];
    int col = node->untried_moves[idx][1];

    // Retirer le coup de la liste des non essayés (swap avec le dernier)
    node->untried_moves[idx][0] = node->untried_moves[node->num_untried - 1][0];
    node->untried_moves[idx][1] = node->untried_moves[node->num_untried - 1][1];
    node->num_untried--;

    // Déterminer le joueur qui va jouer ce coup
    // Le joueur suivant est l'opposé de celui qui a joué le dernier coup
    char next_player;
    if (node->player == PLAYER1) next_player = PLAYER2;
    else if (node->player == PLAYER2) next_player = PLAYER1;
    else next_player = PLAYER1; // Racine : PLAYER1 commence (sera ajusté)

    // Créer le nouvel état
    HexGame new_state = node->state;
    new_state.grid[row][col] = next_player;

    // Créer le nœud enfant
    MCTSNode *child = mcts_create_node(&new_state, row, col, next_player, node);
    node->children[node->num_children++] = child;

    return child;
}

// Phase 3 : Simulation (Rollout) — joue une partie aléatoire
char mcts_simulate(HexGame *game, char current_player) {
    HexGame sim = *game;

    // Vérifier si la partie est déjà terminée
    char winner = mcts_check_winner(&sim);
    if (winner != EMPTY) return winner;

    // Créer la liste des cases vides
    int empty_cells[MAX_SIZE * MAX_SIZE][2];
    int num_empty = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (sim.grid[i][j] == EMPTY) {
                empty_cells[num_empty][0] = i;
                empty_cells[num_empty][1] = j;
                num_empty++;
            }
        }
    }

    // Mélanger les cases vides (Fisher-Yates)
    for (int i = num_empty - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp0 = empty_cells[i][0], tmp1 = empty_cells[i][1];
        empty_cells[i][0] = empty_cells[j][0]; empty_cells[i][1] = empty_cells[j][1];
        empty_cells[j][0] = tmp0; empty_cells[j][1] = tmp1;
    }

    // Remplir le plateau avec des coups alternés
    char player = current_player;
    for (int k = 0; k < num_empty; k++) {
        sim.grid[empty_cells[k][0]][empty_cells[k][1]] = player;
        
        // Vérifier la victoire après chaque coup
        winner = mcts_check_winner(&sim);
        if (winner != EMPTY) return winner;

        player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    }

    // En théorie, au Hex, il y a toujours un gagnant
    // En dernier recours, évaluer la position
    return mcts_check_winner(&sim);
}

// Phase 4 : Rétropropagation — remonte le résultat dans l'arbre
void mcts_backpropagate(MCTSNode *node, char winner, char root_player) {
    while (node != NULL) {
        node->visits++;
        // Du point de vue du joueur qui maximise (root_player)
        if (winner == root_player) {
            node->wins += 1.0;
        } else if (winner != EMPTY) {
            node->wins += 0.0; // Défaite
        } else {
            node->wins += 0.5; // Match nul (ne devrait pas arriver au Hex)
        }
        node = node->parent;
    }
}

// Fonction principale MCTS : trouve le meilleur coup
void best_move_mcts(HexGame *game, char player, int *bestRow, int *bestCol, int turn) {

    // Coups aléatoires en début de partie
    if (turn < RANDTOUR) {
        play_random_move(game, bestRow, bestCol);
        return;
    }

    // Créer la racine de l'arbre
    // Le nœud racine a comme "player" le joueur précédent
    // car l'expansion jouera le joueur suivant
    char prev_player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    MCTSNode *root = mcts_create_node(game, -1, -1, prev_player, NULL);

    // Itérations MCTS
    for (int iter = 0; iter < MCTS_ITERATIONS; iter++) {
        // 1. Sélection
        MCTSNode *node = mcts_select(root);

        // Vérifier si la partie est terminée à ce nœud
        char winner = mcts_check_winner(&node->state);

        if (winner == EMPTY && node->num_untried > 0) {
            // 2. Expansion
            node = mcts_expand(node);
        }

        // 3. Simulation
        char next_player = (node->player == PLAYER1) ? PLAYER2 : PLAYER1;
        if (winner == EMPTY) {
            winner = mcts_simulate(&node->state, next_player);
        }

        // 4. Rétropropagation
        mcts_backpropagate(node, winner, player);
    }

    // Choisir le coup avec le plus de visites (plus robuste que le meilleur taux)
    int best_visits = -1;
    *bestRow = -1;
    *bestCol = -1;

    for (int i = 0; i < root->num_children; i++) {
        MCTSNode *child = root->children[i];
        if (child->visits > best_visits) {
            best_visits = child->visits;
            *bestRow = child->move_row;
            *bestCol = child->move_col;
        } else if (child->visits == best_visits && (rand() % 2 == 0)) {
            // Choix aléatoire en cas d'égalité
            *bestRow = child->move_row;
            *bestCol = child->move_col;
        }
    }

    // Si aucun coup n'a été trouvé (ne devrait pas arriver), jouer aléatoirement
    if (*bestRow == -1 || *bestCol == -1) {
        play_random_move(game, bestRow, bestCol);
    }

    // Libérer l'arbre
    mcts_free_tree(root);
}
