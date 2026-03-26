#include <iostream>
#include <chrono>
#include <iomanip>
#include <random>
#include <windows.h>
#include <psapi.h>
#include <immintrin.h>   // _mm_malloc / _mm_free

using namespace std;

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
    int N = 0;
    cout << "Un nombre entier ? : ";
    cin >> N;
    if (N <= 0) {
        cerr << "N doit etre un entier strictement positif." << endl;
        return 1;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Optimisation 3 : alignement mémoire sur 64 octets
    // _mm_malloc(taille_en_bytes, alignement)
    // 64 octets = taille d'une ligne de cache CPU
    // AVX512 charge 64 bytes en une seule instruction
    // AVX2   charge 32 bytes en une seule instruction
    // Sans alignement → le CPU doit faire 2 chargements au lieu d'1
    // ─────────────────────────────────────────────────────────────────────────
    float* A  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* B  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* BT = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* C  = (float*)_mm_malloc(N * N * sizeof(float), 64);

    if (!A || !B || !BT || !C) {
        cerr << "Erreur d'allocation memoire." << endl;
        return 1;
    }

    // Générateur aléatoire (meilleur que srand/rand — slide 73)
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(1.0f, 100.0f);

    // Remplir A et B avec des valeurs aléatoires
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
            C[i * N + j] = 0.0f;
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

    // Snapshot CPU avant le calcul
    double cpu_before = get_cpu_time_s();

    // Mesure du temps — uniquement le calcul C = A * B
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    // ─────────────────────────────────────────────────────────────────────────
    // Calcul C = A * B avec optimisations 1 + 2 + 3
    // Ligne i de A  ×  Ligne j de BT  (et non colonne j de B)
    // float aligné → FMAD + chargement AVX optimal en une instruction
    // ─────────────────────────────────────────────────────────────────────────
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float somme = 0.0f;
            for (int k = 0; k < N; k++) {
                somme += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = somme;
        }
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    chrono::duration<double> duree_s         = end - start;
    chrono::duration<double, milli> duree_ms = end - start;

    // Snapshot CPU après le calcul
    double cpu_after = get_cpu_time_s();

    double cpu_percent = ((cpu_after - cpu_before) / duree_s.count()) * 100.0;

    PROCESS_MEMORY_COUNTERS pmc{};
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    double mem_mb = pmc.PeakWorkingSetSize / (1024.0 * 1024.0);

    cout << "Duree du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    cout << "Duree du calcul C = A * B : " << duree_s.count()  << " s"  << endl;
    cout << "CPU utilise : " << fixed << setprecision(2) << cpu_percent << " %" << endl;
    cout.unsetf(ios::fixed);
    cout << "Memoire utilisee : " << mem_mb << " Mo" << endl;

    // ─────────────────────────────────────────────────────────────────────────
    // Désallocation — _mm_free OBLIGATOIRE à la place de delete[]
    // car mémoire allouée avec _mm_malloc
    // ─────────────────────────────────────────────────────────────────────────
    _mm_free(A);
    _mm_free(B);
    _mm_free(BT);
    _mm_free(C);

    return 0;
}