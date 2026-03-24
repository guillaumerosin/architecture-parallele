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

----------------------------------------------------------------


TP 2 Matrix: Multithreading
Réalisation d'un programme qui : 
1.  Fait la même chose que l’exercice 1
2. Le calcul est cette fois réalisé par des threads
3. Instanciation d’un thread par case de la matrice C

g++ main.cpp -std=c++20 -o main   pour compiler avec le compilateur c++20

-----------------------------------------------------------------
 
TP 4: Matrix :Veuillez réaliser un programme qui :
1. Fait la même chose que l’exercice 1
2. Le calcul est cette fois réalisé par des threads
3. Le nombre de threads à instancier est demandé à l’utilisateur
Version A du TP : distribution alternée des threads :
Version A du TP : distribution contiguë des threads :
Conservez bien votre programme, car vous allez en avoir besoin plus tard (dossier à réaliser)
[0.25T, 0.5T, 0.75T, T, 1.5T, 2T] où T= Threads de votre CPU


# TP Matrix 3 — Multiplication matricielle multithreadée (un thread par ligne)

## Contexte du cours

Ce TP fait partie du chapitre **Architectures parallèles — Multithreading** (Systèmes temps réel).  
Il s'appuie sur deux blocs de matière : le rappel C/C++ (pointeurs, allocation dynamique) et le multithreading en C++11/20.

---

## Ce que le TP demande (slide 51)

> Réaliser un programme qui :
> 1. Demande à l'utilisateur une valeur entière **N**
> 2. Alloue dynamiquement 3 matrices (A, B, C) d'entiers de taille N×N
> 3. Remplit A et B de valeurs (aléatoires ou non)
> 4. Effectue la multiplication C = A×B **via des threads**
> 5. **Un thread par ligne de la matrice C** (c'est ce qui distingue le TP3 du TP2 qui lui crée un thread par case)
> 6. Affiche la durée du calcul (et exclusivement du calcul)
> 7. Désalloue proprement la mémoire

---

## Rappels du cours utilisés dans ce TP

### 1. Allocation dynamique en C++ (slides 17–19)

Les matrices sont des tableaux 1D représentant une grille 2D.  
L'accès à l'élément ligne `i`, colonne `j` se fait par `tableau[i * N + j]`.

```cpp
int *A = new int[N * N];   // alloue N*N entiers
// ...
delete[] A;                // libère la mémoire (OBLIGATOIRE)
```

> ⚠️ Le cours insiste : **toujours libérer la mémoire** et **ne jamais mélanger `new`/`free` ou `malloc`/`delete`**.

---

### 2. Mesurer le temps (slide 23)

```cpp
#include <chrono>

auto start = std::chrono::steady_clock::now();
// ... calcul ...
auto end   = std::chrono::steady_clock::now();

std::chrono::duration<double, std::milli> duree = end - start;
std::cout << duree.count() << " ms" << std::endl;
```

Le cours demande de mesurer **uniquement** le calcul, pas l'allocation ni l'affichage.

---

### 3. std::thread — Multithreading C++11 (slides 24–26)

Un thread est une unité d'exécution indépendante au sein d'un même processus.  
Tous les threads d'un même programme **partagent la mémoire**.

```cpp
#include <thread>

void ma_fonction(int parametre) { /* ... */ }

std::thread t(ma_fonction, 42);  // crée et démarre le thread
t.join();                         // attend que le thread se termine
```

**`join()`** est indispensable : sans lui, le programme principal peut se terminer
avant les threads, causant un comportement indéfini.

---

### 4. Passage de paramètres par pointeur (slides 5–10)

Les threads reçoivent les matrices par pointeur.  
Passer un pointeur permet au thread de **modifier directement la mémoire partagée**
(ici la matrice C), sans copier les données.

```cpp
// Le thread reçoit l'adresse de A, B, C — il peut lire/écrire directement
void compute_line(const int* A, const int* B, int* C, int N, int i) { ... }
```

Ce mécanisme est expliqué dans le cours via le **passage par variable** (vs passage par valeur).

---

### 5. Pourquoi un thread par ligne ? (slides 50–52)

- **TP2** : un thread par **case** de C → N² threads, overhead énorme pour de grands N
- **TP3** : un thread par **ligne** de C → N threads, chaque thread calcule N produits scalaires
- **TP4** : nombre de threads choisi par l'utilisateur, avec distribution alternée ou contiguë

Plus le grain de parallélisme est fin (TP2), plus le coût de création/synchronisation des threads
est élevé par rapport au travail utile. Le TP3 est un bon compromis.

---

### 6. Pas de mutex ici — pourquoi ? (slides 31–37)

Le cours explique le problème de **data race** : quand deux threads accèdent
à la même variable en même temps, le résultat est imprévisible.

Dans ce TP, **chaque thread écrit sur une ligne différente de C**.  
Il n'y a donc **aucun accès concurrent** à la même case mémoire → pas besoin de mutex.

Si deux threads écrivaient sur la même case (comme en TP2 mal conçu), il faudrait protéger
avec un `std::mutex` :

```cpp
std::mutex mtx;
mtx.lock();
C[i][j] += ...;   // section critique
mtx.unlock();
```

---

## Structure du code

```
main()
 ├── Lecture de N
 ├── Allocation de A, B, C  (new int[N*N])
 ├── Remplissage de A et B  (valeurs aléatoires)
 ├── start = chrono::now()
 ├── Création de N threads  (un par ligne i de C)
 │    └── chaque thread appelle compute_line(A, B, C, N, i)
 ├── join() sur tous les threads
 ├── end = chrono::now()  →  affichage durée
 └── delete[] A, B, C
```

```
compute_line(A, B, C, N, i)
 └── pour chaque colonne j :
      └── C[i*N+j] = somme sur k de A[i*N+k] * B[k*N+j]
```

---

## Compilation et exécution

```bash
# Compiler (C++20 recommandé par le cours, C++11 minimum)
g++ -std=c++20 -O2 -pthread main.cpp -o tp3

# Exécuter
./tp3
# Entre une valeur entier N: 500
```

> Le flag `-pthread` est nécessaire pour linker la bibliothèque des threads sous Linux.

---

## Points d'attention signalés par le cours

| Problème | Explication (cours) | Solution dans ce TP |
|----------|---------------------|---------------------|
| Fuite mémoire | Slide 19 : toujours libérer | `delete[] A/B/C` en fin de main |
| Thread-safety du générateur aléatoire | Slide 73 : `rand()` non thread-safe | Le remplissage se fait **avant** les threads |
| Deadlock | Slide 35 : deux tâches s'attendent | Pas de mutex ici → pas de risque |
| Segmentation fault | Slide 20 : accès mémoire non alloué | Vérification que N > 0 avant allocation |
| `join()` oublié | Slide 27 : programme ne termine jamais | `join()` dans une boucle sur le vecteur de threads |

---

## Résumé des concepts du cours mobilisés

```
Rappel C/C++                    Multithreading
─────────────────────           ──────────────────────────────
• Adresses & pointeurs          • std::thread (C++11)
• Passage par variable          • join()
• new / delete[]                • Pas de data race si zones disjointes
• sizeof / accès tableau 1D     • std::chrono pour mesurer
• Allocation dynamique N×N      • Un thread = une ligne de C
```