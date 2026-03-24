#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <sys/resource.h>

int main() {
    int N;
    std::cout << "Entre une valeur N:\n";
    std::cin >> N;

    // Générateur de nombres aléatoires
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(1, 10);

    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);  // ✅ point-virgule
            B[i * N + j] = dis(gen);  // ✅ point-virgule
        }
    }

    // Snapshot CPU avant calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Chrono start
    auto start = std::chrono::steady_clock::now();

    // Multiplication C = A x B
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }

    // Chrono end
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duree_s = end - start;

    // Snapshot CPU après calcul
    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    // Calcul du % CPU
    double cpu_before =                                         // ✅ noms distincts
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =                                          // ✅ corrigé
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
    delete[] C;

    return 0;
}