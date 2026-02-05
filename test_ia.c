#include <time.h>
#include <string.h>
#include "hex.h"

#define NB_PARTIES 50

// Types de joueurs pour le test
#define TEST_RANDOM 0
#define TEST_MINIMAX 1
#define TEST_ALPHABETA 2

const char* type_name(int type) {
    switch(type) {
        case TEST_RANDOM: return "Random";
        case TEST_MINIMAX: return "Minimax";
        case TEST_ALPHABETA: return "AlphaBeta";
        default: return "Inconnu";
    }
}

// Simule un joueur humain qui joue aléatoirement
void play_random(HexGame *game, int *row, int *col) {
    play_random_move(game, row, col);
}

// Joue une partie complète et retourne le gagnant (1 ou 2)
// Retourne aussi le nombre de tours et le temps total
int play_game(int type1, int type2, int *nb_tours, double *temps_j1, double *temps_j2) {
    HexGame game;
    init_game(&game);
    
    int turn = 0;
    *temps_j1 = 0.0;
    *temps_j2 = 0.0;
    
    while (1) {
        char current_player = (turn % 2 == 0) ? PLAYER1 : PLAYER2;
        int current_type = (turn % 2 == 0) ? type1 : type2;
        
        int row = -1, col = -1;
        double start = omp_get_wtime();
        
        if (current_type == TEST_RANDOM) {
            play_random(&game, &row, &col);
        } else if (current_type == TEST_MINIMAX) {
            best_move_minimax(&game, current_player, &row, &col, turn);
        } else if (current_type == TEST_ALPHABETA) {
            best_move_alphabeta(&game, current_player, &row, &col, turn);
        }
        
        double end = omp_get_wtime();
        double elapsed = end - start;
        
        if (turn % 2 == 0) {
            *temps_j1 += elapsed;
        } else {
            *temps_j2 += elapsed;
        }
        
        if (row == -1 || col == -1) {
            // Erreur, match nul (ne devrait pas arriver)
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
        
        // Sécurité : éviter boucle infinie
        if (turn > BOARD_SIZE * BOARD_SIZE) {
            *nb_tours = turn;
            return 0;
        }
    }
}

// Structure pour stocker les résultats
typedef struct {
    int type1;
    int type2;
    int victoires_j1;
    int victoires_j2;
    int matchs_nuls;
    double temps_total_j1;
    double temps_total_j2;
    int tours_total;
} ResultatMatch;

// Effectue une série de parties entre deux types d'IA
ResultatMatch run_series(int type1, int type2, int nb_parties) {
    ResultatMatch res;
    res.type1 = type1;
    res.type2 = type2;
    res.victoires_j1 = 0;
    res.victoires_j2 = 0;
    res.matchs_nuls = 0;
    res.temps_total_j1 = 0.0;
    res.temps_total_j2 = 0.0;
    res.tours_total = 0;
    
    printf("  Test: %s (J1) vs %s (J2) - %d parties\n", 
           type_name(type1), type_name(type2), nb_parties);
    
    double temps_cumule = 0.0; // Pour estimer le temps restant
    
    for (int i = 0; i < nb_parties; i++) {
        int nb_tours;
        double temps_j1, temps_j2;
        
        int gagnant = play_game(type1, type2, &nb_tours, &temps_j1, &temps_j2);
        
        if (gagnant == 1) {
            res.victoires_j1++;
        } else if (gagnant == 2) {
            res.victoires_j2++;
        } else {
            res.matchs_nuls++;
        }
        
        res.temps_total_j1 += temps_j1;
        res.temps_total_j2 += temps_j2;
        res.tours_total += nb_tours;
        
        // Calcul du temps pour cette partie
        double temps_partie = temps_j1 + temps_j2;
        temps_cumule += temps_partie;
        
        // Estimation du temps restant
        double temps_moyen = temps_cumule / (i + 1);
        int parties_restantes = nb_parties - (i + 1);
        double temps_restant = temps_moyen * parties_restantes;
        
        // Affichage progression APRÈS la partie (donc i+1 parties terminées)
        int pourcentage = ((i + 1) * 100) / nb_parties;
        
        char temps_total_str[50], temps_restant_str[50];
        
        // Formattage temps total
        if (temps_cumule >= 60) sprintf(temps_total_str, "Total: %.1f min", temps_cumule / 60.0);
        else sprintf(temps_total_str, "Total: %.0f sec", temps_cumule);
        
        // Formattage temps restant
        if (temps_restant >= 60) sprintf(temps_restant_str, "Reste: %.1f min", temps_restant / 60.0);
        else sprintf(temps_restant_str, "Reste: %.0f sec", temps_restant);

        printf("\r  Progression: %d/%d (%d%%) - J1:%d J2:%d - %s - %s   ", 
               i + 1, nb_parties, pourcentage, res.victoires_j1, res.victoires_j2, temps_total_str, temps_restant_str);
        
        fflush(stdout);
    }
    printf("\n");
    
    return res;
}

// Écrit les résultats dans un fichier
void write_results(FILE *f, ResultatMatch *results, int nb_results) {
    fprintf(f, "================================================================================\n");
    fprintf(f, "                    RÉSULTATS DES TESTS IA - JEU HEX %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    fprintf(f, "================================================================================\n");
    fprintf(f, "Paramètres:\n");
    fprintf(f, "  - Taille du plateau: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    fprintf(f, "  - Profondeur Minimax: %d\n", PROFMINIMAX);
    fprintf(f, "  - Profondeur Alpha-Beta: %d\n", PROFALPHABETA);
    fprintf(f, "  - Tours aléatoires au début: %d\n", RANDTOUR);
    fprintf(f, "  - Nombre de parties par test: %d\n", NB_PARTIES);
    fprintf(f, "================================================================================\n\n");
    
    for (int i = 0; i < nb_results; i++) {
        ResultatMatch r = results[i];
        
        fprintf(f, "--------------------------------------------------------------------------------\n");
        fprintf(f, "TEST %d: %s (Joueur 1 - Rouge) vs %s (Joueur 2 - Bleu)\n", 
                i + 1, type_name(r.type1), type_name(r.type2));
        fprintf(f, "--------------------------------------------------------------------------------\n");
        
        fprintf(f, "\n  VICTOIRES:\n");
        fprintf(f, "    Joueur 1 (%s): %d (%.1f%%)\n", 
                type_name(r.type1), r.victoires_j1, 
                (r.victoires_j1 * 100.0) / NB_PARTIES);
        fprintf(f, "    Joueur 2 (%s): %d (%.1f%%)\n", 
                type_name(r.type2), r.victoires_j2, 
                (r.victoires_j2 * 100.0) / NB_PARTIES);
        if (r.matchs_nuls > 0) {
            fprintf(f, "    Matchs nuls: %d\n", r.matchs_nuls);
        }
        
        fprintf(f, "\n  TEMPS D'EXÉCUTION:\n");
        fprintf(f, "    Joueur 1 total: %.2f sec (moyenne: %.4f sec/partie)\n", 
                r.temps_total_j1, r.temps_total_j1 / NB_PARTIES);
        fprintf(f, "    Joueur 2 total: %.2f sec (moyenne: %.4f sec/partie)\n", 
                r.temps_total_j2, r.temps_total_j2 / NB_PARTIES);
        
        fprintf(f, "\n  STATISTIQUES:\n");
        fprintf(f, "    Nombre moyen de tours par partie: %.1f\n", 
                (double)r.tours_total / NB_PARTIES);
        
        // Déterminer le gagnant de la série
        fprintf(f, "\n  CONCLUSION: ");
        if (r.victoires_j1 > r.victoires_j2) {
            fprintf(f, "%s (J1) GAGNE la série avec %d victoires!\n", 
                    type_name(r.type1), r.victoires_j1);
        } else if (r.victoires_j2 > r.victoires_j1) {
            fprintf(f, "%s (J2) GAGNE la série avec %d victoires!\n", 
                    type_name(r.type2), r.victoires_j2);
        } else {
            fprintf(f, "ÉGALITÉ entre les deux IA!\n");
        }
        fprintf(f, "\n");
    }
    
    // Résumé final
    fprintf(f, "================================================================================\n");
    fprintf(f, "                              RÉSUMÉ GLOBAL\n");
    fprintf(f, "================================================================================\n\n");
    
    // Compter les victoires totales par type d'IA
    int victoires_random = 0, victoires_minimax = 0, victoires_alphabeta = 0;
    int parties_random = 0, parties_minimax = 0, parties_alphabeta = 0;
    double temps_random = 0, temps_minimax = 0, temps_alphabeta = 0;
    
    for (int i = 0; i < nb_results; i++) {
        ResultatMatch r = results[i];
        
        // Joueur 1
        if (r.type1 == TEST_RANDOM) {
            victoires_random += r.victoires_j1;
            parties_random += NB_PARTIES;
            temps_random += r.temps_total_j1;
        } else if (r.type1 == TEST_MINIMAX) {
            victoires_minimax += r.victoires_j1;
            parties_minimax += NB_PARTIES;
            temps_minimax += r.temps_total_j1;
        } else if (r.type1 == TEST_ALPHABETA) {
            victoires_alphabeta += r.victoires_j1;
            parties_alphabeta += NB_PARTIES;
            temps_alphabeta += r.temps_total_j1;
        }
        
        // Joueur 2
        if (r.type2 == TEST_RANDOM) {
            victoires_random += r.victoires_j2;
            parties_random += NB_PARTIES;
            temps_random += r.temps_total_j2;
        } else if (r.type2 == TEST_MINIMAX) {
            victoires_minimax += r.victoires_j2;
            parties_minimax += NB_PARTIES;
            temps_minimax += r.temps_total_j2;
        } else if (r.type2 == TEST_ALPHABETA) {
            victoires_alphabeta += r.victoires_j2;
            parties_alphabeta += NB_PARTIES;
            temps_alphabeta += r.temps_total_j2;
        }
    }
    
    fprintf(f, "Performance globale par type d'IA:\n\n");
    
    if (parties_random > 0) {
        fprintf(f, "  RANDOM:\n");
        fprintf(f, "    Victoires: %d/%d (%.1f%%)\n", 
                victoires_random, parties_random, 
                (victoires_random * 100.0) / parties_random);
        fprintf(f, "    Temps moyen/partie: %.4f sec\n\n", temps_random / parties_random);
    }
    
    if (parties_minimax > 0) {
        fprintf(f, "  MINIMAX (profondeur %d):\n", PROFMINIMAX);
        fprintf(f, "    Victoires: %d/%d (%.1f%%)\n", 
                victoires_minimax, parties_minimax, 
                (victoires_minimax * 100.0) / parties_minimax);
        fprintf(f, "    Temps moyen/partie: %.4f sec\n\n", temps_minimax / parties_minimax);
    }
    
    if (parties_alphabeta > 0) {
        fprintf(f, "  ALPHA-BETA (profondeur %d):\n", PROFALPHABETA);
        fprintf(f, "    Victoires: %d/%d (%.1f%%)\n", 
                victoires_alphabeta, parties_alphabeta, 
                (victoires_alphabeta * 100.0) / parties_alphabeta);
        fprintf(f, "    Temps moyen/partie: %.4f sec\n\n", temps_alphabeta / parties_alphabeta);
    }
    
    // Avantage Joueur 1 vs Joueur 2
    fprintf(f, "--------------------------------------------------------------------------------\n");
    fprintf(f, "Avantage positionnel (Joueur 1 vs Joueur 2):\n");
    fprintf(f, "--------------------------------------------------------------------------------\n");
    
    int total_j1 = 0, total_j2 = 0;
    for (int i = 0; i < nb_results; i++) {
        total_j1 += results[i].victoires_j1;
        total_j2 += results[i].victoires_j2;
    }
    
    int total_parties = total_j1 + total_j2;
    fprintf(f, "  Joueur 1 (commence): %d victoires (%.1f%%)\n", 
            total_j1, (total_j1 * 100.0) / total_parties);
    fprintf(f, "  Joueur 2 (second):   %d victoires (%.1f%%)\n", 
            total_j2, (total_j2 * 100.0) / total_parties);
    
    if (total_j1 > total_j2) {
        fprintf(f, "\n  => Le Joueur 1 (qui commence) a un avantage de %.1f%%\n",
                ((total_j1 - total_j2) * 100.0) / total_parties);
    } else if (total_j2 > total_j1) {
        fprintf(f, "\n  => Le Joueur 2 (qui joue en second) a un avantage de %.1f%%\n",
                ((total_j2 - total_j1) * 100.0) / total_parties);
    } else {
        fprintf(f, "\n  => Aucun avantage positionnel détecté\n");
    }
    
    fprintf(f, "\n================================================================================\n");
}

int main() {
    srand(time(NULL));
    
    printf("================================================================================\n");
    printf("           TEST AUTOMATIQUE DES IA - JEU HEX %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    printf("================================================================================\n");
    printf("Configuration:\n");
    printf("  - Taille: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    printf("  - Profondeur Minimax: %d\n", PROFMINIMAX);
    printf("  - Profondeur Alpha-Beta: %d\n", PROFALPHABETA);
    printf("  - Nombre de parties par test: %d\n", NB_PARTIES);
    printf("================================================================================\n\n");
    
    ResultatMatch results[10];
    int nb_results = 0;
    
    // Test 1: Random vs Random (pour voir l'avantage du premier joueur)
    printf("[1/9] Random vs Random\n");
    results[nb_results++] = run_series(TEST_RANDOM, TEST_RANDOM, NB_PARTIES);
    
    // Test 2: Minimax vs Random
    printf("[2/9] Minimax vs Random\n");
    results[nb_results++] = run_series(TEST_MINIMAX, TEST_RANDOM, NB_PARTIES);
    
    // Test 3: Random vs Minimax
    printf("[3/9] Random vs Minimax\n");
    results[nb_results++] = run_series(TEST_RANDOM, TEST_MINIMAX, NB_PARTIES);
    
    // Test 4: Alpha-Beta vs Random
    printf("[4/9] Alpha-Beta vs Random\n");
    results[nb_results++] = run_series(TEST_ALPHABETA, TEST_RANDOM, NB_PARTIES);
    
    // Test 5: Random vs Alpha-Beta
    printf("[5/9] Random vs Alpha-Beta\n");
    results[nb_results++] = run_series(TEST_RANDOM, TEST_ALPHABETA, NB_PARTIES);
    
    // Test 6: Minimax vs Alpha-Beta
    printf("[6/9] Minimax vs Alpha-Beta\n");
    results[nb_results++] = run_series(TEST_MINIMAX, TEST_ALPHABETA, NB_PARTIES);
    
    // Test 7: Alpha-Beta vs Minimax
    printf("[7/9] Alpha-Beta vs Minimax\n");
    results[nb_results++] = run_series(TEST_ALPHABETA, TEST_MINIMAX, NB_PARTIES);
    
    // Test 8: Minimax vs Minimax
    printf("[8/9] Minimax vs Minimax\n");
    results[nb_results++] = run_series(TEST_MINIMAX, TEST_MINIMAX, NB_PARTIES);
    
    // Test 9: Alpha-Beta vs Alpha-Beta
    printf("[9/9] Alpha-Beta vs Alpha-Beta\n");
    results[nb_results++] = run_series(TEST_ALPHABETA, TEST_ALPHABETA, NB_PARTIES);
    
    // Écriture des résultats dans un fichier
    char filename[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(filename, "test_results_%d%02d%02d_%02d%02d%02d.txt",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE *f = fopen(filename, "w");
    if (f != NULL) {
        write_results(f, results, nb_results);
        fclose(f);
        printf("\n================================================================================\n");
        printf("Résultats sauvegardés dans: %s\n", filename);
        printf("================================================================================\n");
    } else {
        printf("Erreur: impossible de créer le fichier de résultats.\n");
        // Afficher quand même à l'écran
        write_results(stdout, results, nb_results);
    }
    
    return 0;
}
