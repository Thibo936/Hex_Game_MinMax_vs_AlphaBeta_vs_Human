Thibaut Castillo L3B

# Projet Hex - Algorithmique Avancée

Ce projet est une implémentation du jeu de Hex en langage C, permettant des affrontements entre :
- humain vs humain
- humain vs IA (Minimax ou Alpha-Beta)
- IA vs IA (Minimax contre Alpha-Beta ou Alpha-Beta contre Minimax ou Alpha-Beta contre Alpha-Beta)

## Structures
    HexGame : Contient la grille de jeu (`SIZE x SIZE`). Elle est donnée en référence à toutes les fonctions pour manipuler l'état du plateau de jeu.

## Fonctions hex_game.c
    - init_game : Initialise le plateau avec des cases vides (`.`). Baser sur un plateau de puissance 4 avec un espace en plus à chaque ligne pour l'effet hexagonal.
    - print_board : Affiche le plateau en format hexagonal avec des couleurs (Rouge pour J1, Bleu pour J2).
    - min_distance (BFS) : Calcule la distance minimale pour qu'un joueur relie ses deux bords.
    - eval : Calcule qui a l'avantage en regardant quel joueur est le plus proche de la victoire.

## Algorithmes d'IA
    - minimax : Exploration récursive de tout l'arbre de jeu.
    - alphabeta : Minimax optimisée utilisant l'élagage pour ignorer les branches inutiles.
    - best_move_alphabeta et best_move_minimax : Trouvent le meilleur coup à jouer en parallélisant avec OpenMP.

## Paramètres modifiables dans hex.h
    - SIZE : Taille du plateau (6x6).
    - PROFMINIMAX : Profondeur de recherche pour l'algorithme Minimax.
    - PROFALPHABETA : Profondeur de recherche pour l'algorithme Alpha-Bêta.
    - MAX_THREADS : Nombre maximum de threads pour OpenMP.

## Problèmes Rencontrés
    - La Parallélisation de l'Alpha-Beta car l'élagage utilise les résultats des branches précédentes.
    - Quelque problèmes de gestion de mémoire pour éviter les conflits avec la parallélisation.

## Exécution
### Compilation et Lancement

On utilise un `Makefile` avec le flag `-fopenmp` pour la gestion des threads :
```
make

./hex_final
```
