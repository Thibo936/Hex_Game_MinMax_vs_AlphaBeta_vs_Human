# Projet Hex - Algorithmique Avancée

Ce projet est une implémentation du jeu de Hex en langage C, permettant des affrontements entre :
- humain vs humain
- humain vs IA (Minimax, Alpha-Beta, MCTS ou AlphaZero)
- IA vs IA (toutes combinaisons possibles)

## Structures
    HexGame : Contient la grille de jeu (MAX_SIZE x MAX_SIZE avec taille configurable). Elle est donnée en référence à toutes les fonctions pour manipuler l'état du plateau de jeu.

## Tailles de plateau supportées
    - 9x9 : Taille recommandée pour des parties rapides
    - 11x11 : Taille standard pour le jeu de Hex
    - 14x14 : Taille pour des parties plus complexes

## Fonctions hex_game.c
    - init_game : Initialise le plateau avec des cases vides (`.`). Basé sur un plateau avec un espace en plus à chaque ligne pour l'effet hexagonal.
    - print_board : Affiche le plateau en format hexagonal avec des couleurs (Rouge pour J1, Bleu pour J2).
    - min_distance (BFS) : Calcule la distance minimale pour qu'un joueur relie ses deux bords.
    - eval : Calcule qui a l'avantage en regardant quel joueur est le plus proche de la victoire.

## Algorithmes d'IA
    - minimax : Exploration récursive de tout l'arbre de jeu.
    - alphabeta : Minimax optimisée utilisant l'élagage pour ignorer les branches inutiles.
    - mcts : Monte Carlo Tree Search - exploration de l'arbre avec simulations aléatoires.
    - alphazero : AlphaZero simplifié - MCTS guidé par des heuristiques (version sans réseau de neurones).
    - best_move_* : Trouvent le meilleur coup à jouer, avec parallélisation OpenMP pour Minimax et Alpha-Beta.

## Paramètres modifiables dans hex.h
    - MAX_SIZE : Taille maximale du plateau supportée (14x14).
    - BOARD_SIZE : Variable globale définie au runtime selon le choix de l'utilisateur.
    - PROFMINIMAX : Profondeur de recherche pour l'algorithme Minimax.
    - PROFALPHABETA : Profondeur de recherche pour l'algorithme Alpha-Bêta.
    - MCTS_ITERATIONS : Nombre d'itérations pour MCTS (50000 par défaut).
    - ALPHAZERO_ITERATIONS : Nombre d'itérations pour AlphaZero (30000 par défaut).
    - MAX_THREADS : Nombre maximum de threads pour OpenMP.

## Problèmes Résolus
    - La Parallélisation de l'Alpha-Beta car l'élagage utilise les résultats des branches précédentes.
    - Quelques problèmes de gestion de mémoire pour éviter les conflits avec la parallélisation.
    - MCTS ne trouvait pas de premier coup quand il jouait en premier (résolu avec un fallback).

## Exécution
### Compilation et Lancement

On utilise un `Makefile` avec le flag `-fopenmp` pour la gestion des threads :
```
make

./hex_game
```

### Options de jeu
Au lancement, vous pouvez choisir :
1. La taille du plateau (9x9, 11x11 ou 14x14)
2. Le type de joueur 1 (Humain, Minimax, Alpha-Beta, MCTS ou AlphaZero)
3. Le type de joueur 2 (Humain, Minimax, Alpha-Beta, MCTS ou AlphaZero)

## Notes sur AlphaZero
L'implémentation AlphaZero est une version simplifiée inspirée de l'algorithme original.
L'AlphaZero complet nécessiterait :
- Un réseau de neurones profond (Deep Neural Network)
- Un apprentissage par renforcement via auto-jeu (Self-Play)
- Une optimisation continue des poids du réseau

Cette version utilise à la place :
- MCTS avec sélection PUCT (Predictor + UCT)
- Heuristiques pour les fonctions de politique et de valeur
- Évaluation basée sur les distances minimales au lieu de simulations complètes
