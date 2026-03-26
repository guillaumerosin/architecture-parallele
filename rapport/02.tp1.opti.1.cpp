#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <sys/resource.h>

int main() {
    int N;
    std::cout << "Entre une valeur N:\n";
    std::cin >> N;

    // Mon générateur de nombres aléatoires
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(1, 100);

    int *A  = new int[N * N];
    int *B  = new int[N * N];
    int *BT = new int[N * N];   // Optimisation 1
    int *C  = new int[N * N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
            C[i * N + j] = 0;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Optimisation 1 : calcul de la transposée de B — AVANT le chrono
    // BT[j][k] = B[k][j]  →  BT[j*N+k] = B[k*N+j]
    // La colonne j de B devient la ligne j de BT
    // ─────────────────────────────────────────────────────────────────────────
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            BT[i * N + j] = B[j * N + i];
        }
    }

    // Snapshot CPU avant calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Chrono start
    auto start = std::chrono::steady_clock::now();

    // ─────────────────────────────────────────────────────────────────────────
    // Multiplication C = A x B avec Optimisation 1
    // Ligne i de A  x  Ligne j de BT  (et non colonne j de B)
    // BT[j*N+k] = lecture séquentielle → moins de cache miss
    // ─────────────────────────────────────────────────────────────────────────
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }

    // Fin du chrono
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duree_s = end - start;

    // Snapshot CPU après calcul
    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    // Calcul du % CPU
    double cpu_before =
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =
        usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
        usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

    double cpu_time    = cpu_after - cpu_before;
    double cpu_percent = (cpu_time / duree_s.count()) * 100.0;

    // Mémoire en Mo
    double mem_mb = usage_after.ru_maxrss / 1024.0;

    std::cout << "Durée C = A * B : " << duree_s.count() << " s\n";
    std::cout << "CPU utilisé : " << std::fixed << std::setprecision(2)
              << cpu_percent << " %\n";
    std::cout.unsetf(std::ios::fixed);
    std::cout << "Mémoire utilisée : " << mem_mb << " Mo\n";

    // Désallocation
    delete[] A;
    delete[] B;
    delete[] BT;
    delete[] C;

    return 0;
}