#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <windows.h>
#include <psapi.h>

static double get_cpu_time_s() {
    FILETIME creation, exit, kernel, user;
    GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user);
    auto to_sec = [](FILETIME ft) {
        ULARGE_INTEGER ui;
        ui.LowPart  = ft.dwLowDateTime;
        ui.HighPart = ft.dwHighDateTime;
        return ui.QuadPart / 1e7;
    };
    return to_sec(user) + to_sec(kernel);
}

int main() {
    int N;
    std::cout << "Entre une valeur N:" << std::endl;
    std::cin >> N;

    // Mon générateur de nombres aléatoires
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(1, 100);

    int *A  = new int[N * N];
    int *B  = new int[N * N];
    int *BT = new int[N * N];   // transposée de B — Optimisation 1
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
    double cpu_before = get_cpu_time_s();

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
    double cpu_after = get_cpu_time_s();

    double cpu_time    = cpu_after - cpu_before;
    double cpu_percent = (cpu_time / duree_s.count()) * 100.0;

    // Mémoire en Mo
    PROCESS_MEMORY_COUNTERS pmc{};
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    double mem_mb = pmc.PeakWorkingSetSize / (1024.0 * 1024.0);

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