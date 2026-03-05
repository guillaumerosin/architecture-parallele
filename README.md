# architecture-parallele

TP matrix 1 : multiplication matricielle

Réalisation d'un programme qui : 
1. demande à l'utilisateur une valeur entière N 
2. Alloue dynamiquement 3 matrices (A, B et C ) d’entiers et de taille N x N
3. Remplit les matrices A et B de valeurs (aléatoires ou non)
4. Effectue la multiplication matricielle C=A*B
5. Affiche la durée qui a été nécessaire pour le calcul (et exclusivement pour le calcul)
6. N’oubliez pas de désallouer

# prise de note rapide 

Compile : g++ multi-matriciel.cpp -o multi-matriciel




TP 2 Matrix: Multithreading
Réalisation d'un programme qui : 
1.  Fait la même chose que l’exercice 1
2. Le calcul est cette fois réalisé par des threads
3. Instanciation d’un thread par case de la matrice C

g++ main.cpp -std=c++20 -o main   pour compiler avec le compilateur c++20
