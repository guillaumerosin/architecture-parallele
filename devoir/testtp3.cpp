// RealTimeSys_Exo3.cpp — Rosin Guillaume, TP3 - Calcul parallèle (un thread par ligne)
// Contrairement au TP2 (un thread par cellule = N*N threads), ici on crée un thread
// par ligne de la matrice résultat C, soit seulement N threads au total.
// Chaque thread calcule une ligne entière de C = A * B.
// Beaucoup plus raisonnable en termes de surcharge système.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Générateur aléatoire global
// ---------------------------------------------------------------------------
static mt19937 gen(random_device{}());
static uniform_int_distribution<> dis(1, 10);

// ---------------------------------------------------------------------------
// Calcule une ligne entière i de C : C[i][j] = somme(A[i][k] * B[k][j])
// Un thread = une ligne complète (N cellules calculées par thread)
// ---------------------------------------------------------------------------
static void thread_fct(const int* A, const int* B, int* C, int i, int N)
{
    for (int j = 0; j < N; ++j)
    {
        int sum = 0;
        for (int k = 0; k < N; ++k)
            sum += A[i * N + k] * B[k * N + j];
        C[i * N + j] = sum;
    }
}

// ---------------------------------------------------------------------------
// Lance exactement N threads (un par ligne) et attend leur fin
// N threads au lieu de N*N au TP2 → beaucoup moins de surcharge
// ---------------------------------------------------------------------------
static void matriceproduct(const int* A, const int* B, int* C, int N)
{
    vector<thread> threads;
    threads.reserve(N); // on sait exactement combien de threads on va créer

    for (int i = 0; i < N; ++i)
        threads.emplace_back(thread_fct, A, B, C, i, N);

    for (auto& t : threads)
        t.join();
    // Tous les threads sont détruits automatiquement ici (RAII)
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main()
{
    while (true)
    {
        int N;
        cout << "Taille de la matrice (entier > 0) : ";
        if (!(cin >> N) || N <= 0) {
            cout << "Entrée invalide, arrêt." << endl;
            break;
        }

        // --- Allocation flat (cache-friendly, tout contigu en mémoire) ---
        vector<int> A(static_cast<size_t>(N) * N);
        vector<int> B(static_cast<size_t>(N) * N);
        vector<int> C(static_cast<size_t>(N) * N, 0);

        // --- Remplissage aléatoire ---
        for (int& v : A) v = dis(gen);
        for (int& v : B) v = dis(gen);

        cout << "***********************************************" << endl;

        // --- Snapshot CPU avant le calcul ---
        rusage usage_before{};
        getrusage(RUSAGE_SELF, &usage_before);

        auto t_start = chrono::steady_clock::now();
        matriceproduct(A.data(), B.data(), C.data(), N);
        auto t_end = chrono::steady_clock::now();

        // --- Snapshot CPU après le calcul ---
        rusage usage_after{};
        getrusage(RUSAGE_SELF, &usage_after);

        // --- Calcul des métriques ---
        chrono::duration<double, milli> duree_ms = t_end - t_start;
        chrono::duration<double>        duree_s  = t_end - t_start;

        double cpu_before =
            usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
            usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

        double cpu_after =
            usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
            usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

        double cpu_percent = ((cpu_after - cpu_before) / duree_s.count()) * 100.0;
        double mem_mb      = usage_after.ru_maxrss / 1024.0;

        // --- Affichage ---
        cout << "Taille          : " << N << " x " << N << endl;
        cout << "Duree           : " << duree_ms.count() << " ms  |  "
             << duree_s.count()  << " s"  << endl;
        cout << "CPU utilise     : " << fixed << setprecision(2)
             << cpu_percent << " %" << endl;
        cout.unsetf(ios::fixed);
        cout << "Memoire max     : " << mem_mb << " Mo" << endl;
        cout << "Threads lances  : " << N << " (un par ligne)" << endl;
        cout << "-------------------------------------------------" << endl;

        // A, B, C libérés automatiquement ici (RAII)
    }

    return 0;
}