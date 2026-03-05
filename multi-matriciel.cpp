#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>

// générateur aléatoire global pour remplir les matrices
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 100); // valeurs entre 1 et 100

int main(void) {
    using std::cout;
    using std::endl;

    int N;
    std::printf("Entre une valeur entier N:\n");
    if (std::scanf("%d", &N) != 1 || N <= 0) {
        std::fprintf(stderr, "N doit être un entier strictement positif.\n");
        return 1;
    }

    // 2. allouer dynamiquement A, B, C (N x N)
    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    // 3. remplir A et B avec des valeurs aléatoires entre 1 et 100
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }

    // 4. mesurer uniquement le temps de calcul de C = A * B
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duree_ms = end - start;

    // 5. afficher la durée du calcul uniquement
    cout << "Durée du calcul C = A * B : " << duree_ms.count() << " ms" << endl;

    // 6. désallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}


