#include <thread>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <sys/resource.h>
#include <iomanip>

int main(void) {
    int N;

    std::cout << "entre une valeur N:\n";
    std::cin >> N;

    // 1. Allocation dynamique des matrices A, B, C (tableaux 1D de taille N*N)
    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    // Générateur aléatoire pour remplir A et B
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 9);

    // 2. Remplissage des matrices A et B avec des valeurs aléatoires
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }

    // 3. Snapshot CPU avant le calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // 4. Chrono début
    auto start = std::chrono::steady_clock::now();

    // 5. Calcul de C : un thread par case, mais un seul thread à la fois
    // Lambda qui calcule UNE case de C, ne prend que i et j
    auto compute_cell = [A, B, C, N](int i, int j) {
        int sum = 0;
        for (int k = 0; k < N; ++k) {
            sum += A[i * N + k] * B[k * N + j];
        }
        C[i * N + j] = sum;
    };
    std::vector<std::thread> threads;  //je déclare un contenaire qui va stocker mes threads 
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            threads.emplace_back(compute_cell, i, j);  
        }
    }
    for (auto &t : threads)
        t.join();
    // 6. Chrono fin
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duree_s = end - start;

    // 8. Snapshot CPU après le calcul
    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    double cpu_before =
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =
        usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
        usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

    double cpu_time = cpu_after - cpu_before;
    double cpu_percent = (cpu_time / duree_s.count()) * 100.0;

    // 9. Mémoire utilisée (en Mo)
    double mem_mb = usage_after.ru_maxrss / 1024.0;

    std::cout << "Durée pour le calcul  ==> C = A * B : "
              << duree_s.count() << " s" << std::endl;
    std::cout << "CPU utilise : " << std::fixed << std::setprecision(2)
              << cpu_percent << " %" << std::endl;
    std::cout.unsetf(std::ios::fixed);
    std::cout << "Memoire utilisée : " << mem_mb << " Mo" << std::endl;

    // 10. Désallocation des matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}