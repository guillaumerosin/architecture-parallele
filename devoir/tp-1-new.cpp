//Rosin Guillaume, TP 1 - Architecture Parallèle - Calcul séquenciel
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <iomanip>

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

    // snapshot CPU AVANT le calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

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
    std::chrono::duration<double> duree_s = end - start;

    // snapshot CPU APRÈS le calcul
    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    // temps CPU (user + système) AVANT et APRÈS en secondes
    double cpu_before =
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =
        usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
        usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

    double cpu_time = cpu_after - cpu_before;
    double cpu_percent = (cpu_time / duree_s.count()) * 100.0;

    // ru_maxrss en Ko -> Mo
    double mem_mb = usage_after.ru_maxrss / 1024.0;

    // 5. afficher les mesures
    cout << "Durée du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    cout << "Durée du calcul C = A * B : " << duree_s.count() << " s" << endl;
    cout << "CPU utilise : " << std::fixed << std::setprecision(2) << cpu_percent << " %" << std::endl;
    cout.unsetf(std::ios::fixed);
    cout << "Memoire max : " << mem_mb << " Mo" << std::endl;

    // 6. désallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}


