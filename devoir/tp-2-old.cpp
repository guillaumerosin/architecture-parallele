#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>

using namespace std;

static mt19937 gen(random_device{}());
static uniform_int_distribution<> dis(1, 10);


static void thread_fct(const int* A, const int* B, int* C, int i, int j, int N)
{
    int sum = 0;
    for (int k = 0; k < N; ++k)
        sum += A[i * N + k] * B[k * N + j];
    C[i * N + j] = sum;
}

static void matriceproduct(const int* A, const int* B, int* C, int N)
{
    // Réserver tous les threads d'un coup pour éviter les réallocations
    vector<thread> threads;
    threads.reserve(static_cast<size_t>(N) * N);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            threads.emplace_back(thread_fct, A, B, C, i, j, N);

    for (auto& t : threads)
        t.join();
    
}

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

        // --- Allocation flat (cache-friendly) ---
        vector<int> A(static_cast<size_t>(N) * N);
        vector<int> B(static_cast<size_t>(N) * N);
        vector<int> C(static_cast<size_t>(N) * N, 0);

        // --- Remplissage aléatoire ---
        for (int& v : A) v = dis(gen);
        for (int& v : B) v = dis(gen);

        cout << "***********************************************" << endl;

        // --- Snapshot CPU avant ---
        rusage usage_before{};
        getrusage(RUSAGE_SELF, &usage_before);

        auto t_start = chrono::steady_clock::now();
        matriceproduct(A.data(), B.data(), C.data(), N);
        auto t_end = chrono::steady_clock::now();

        // --- Snapshot CPU après ---
        rusage usage_after{};
        getrusage(RUSAGE_SELF, &usage_after);

        // --- Calculs des métriques ---
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
        cout << "Threads lances  : " << N * N << endl;
        cout << "-------------------------------------------------" << endl;

        // A, B, C sont des std::vector → libérés automatiquement ici (RAII)
    }

    return 0;
}