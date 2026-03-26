#include <iostream>
#include <chrono>
#include <iomanip>
#include <random>
#include <sys/resource.h>

using namespace std;

int main() {
    int N = 0;
    cout << "Un nombre entier ? : ";
    cin >> N;
    if (N <= 0) {
        cerr << "N doit etre un entier strictement positif." << endl;
        return 1;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Optimisation 2 : float au lieu de int
    // Le CPU peut utiliser l'instruction FMAD des unités AVX sur des float
    // On passe aussi en tableau 1D (A[i*N+j]) pour une mémoire contiguë
    // ─────────────────────────────────────────────────────────────────────────
    float* A  = new float[N * N];
    float* B  = new float[N * N];
    float* C  = new float[N * N];
    float* BT = new float[N * N];   // transposée de B — Optimisation 1

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
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Mesure du temps — uniquement le calcul C = A * B
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    // ─────────────────────────────────────────────────────────────────────────
    // Calcul C = A * B avec optimisations 1 + 2
    // Ligne i de A  ×  Ligne j de BT  (et non colonne j de B)
    // float → le CPU utilise FMAD des unités AVX automatiquement
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
    chrono::duration<double> duree_s  = end - start;
    chrono::duration<double, milli> duree_ms = end - start;

    // Snapshot CPU après le calcul
    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    double cpu_before =
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =
        usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
        usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

    double cpu_percent = ((cpu_after - cpu_before) / duree_s.count()) * 100.0;
    double mem_mb      = usage_after.ru_maxrss / 1024.0;

    cout << "Duree du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    cout << "Duree du calcul C = A * B : " << duree_s.count()  << " s"  << endl;
    cout << "CPU utilise : " << fixed << setprecision(2) << cpu_percent << " %" << endl;
    cout.unsetf(ios::fixed);
    cout << "Memoire utilisee : " << mem_mb << " Mo" << endl;

    // Désallocation (new → delete[])
    delete[] A;
    delete[] B;
    delete[] BT;
    delete[] C;

    return 0;
}