#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    int N = 0;
    cout << "Un nombre entier ? : ";
    cin >> N;

    // Allocation dynamique de trois matrices N x N
    int** A = new int*[N];
    int** B = new int*[N];
    int** C = new int*[N];

    srand(time(NULL));

    // Initialisation des matrices A et B avec des valeurs aléatoires
    for (int i = 0; i < N; i++) {
        A[i] = new int[N];
        B[i] = new int[N];
        C[i] = new int[N];

        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10 + 1; // Valeur entre 1 et 10
            B[i][j] = rand() % 10 + 1;
            C[i][j] = 0;
        }
    }

    // Mesure du temps de début
    chrono::time_point<chrono::system_clock> start, end;
    start = chrono::system_clock::now();

    // Multiplication matricielle : C = A x B
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int somme = 0;
            for (int k = 0; k < N; k++) {
                somme += A[i][k] * B[k][j];
            }
            C[i][j] = somme;
        }
    }

    // Mesure du temps de fin
    end = chrono::system_clock::now();
    long long int microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Temps d'execution : " << microseconds << " micro sec\n";

    // Libération de la mémoire
    for (int i = 0; i < N; i++) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
