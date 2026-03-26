#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>

using namespace std;

// générateur aléatoire global pour remplir les matrices
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 100);

// Fonction exécutée par chaque thread
// Calcule une ligne entière i de C = A * B
void compute_line(const int* A, const int* B, int* C, int N, int i) {
    for (int j = 0; j < N; j++) {
        int sum = 0;
        for (int k = 0; k < N; k++) {
            sum += A[i * N + k] * B[k * N + j];
        }
        C[i * N + j] = sum;
    }
}

int main() {

    // 1. Demander N à l'utilisateur
    int N = 0;
    std::printf("Entrez la taille N des matrices : ");
    if (std::scanf("%d", &N) != 1 || N <= 0) {
        std::fprintf(stderr, "N doit etre un entier strictement positif.\n");
        return 1;
    }

    // 2. Allouer dynamiquement les 3 matrices A, B, C de taille N*N
    int* A = new int[N * N];
    int* B = new int[N * N];
    int* C = new int[N * N];

    // 3. Remplir A et B avec des valeurs aléatoires entre 1 et 100
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }

    // Snapshot CPU avant calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // 4. Multiplication C = A * B avec un thread par ligne de C
    auto start = chrono::steady_clock::now();

    vector<thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; i++) {
        threads.emplace_back(compute_line, A, B, C, N, i);
    }

    for (int i = 0; i < N; i++) {
        threads[i].join();
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> duree_ms = end - start;
    chrono::duration<double> duree_s = end - start;

    // Snapshot CPU après calcul
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
    double mem_mb = usage_after.ru_maxrss / 1024.0;

    // 5. Afficher les mêmes mesures que tp-matrix-3
    cout << "Durée du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    cout << "Durée du calcul C = A * B : " << duree_s.count() << " s" << endl;
    cout << "CPU utilise : " << fixed << setprecision(2) << cpu_percent << " %" << endl;
    cout.unsetf(std::ios::fixed);
    cout << "Memoire max : " << mem_mb << " Mo" << endl;

    // 6. Désallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}