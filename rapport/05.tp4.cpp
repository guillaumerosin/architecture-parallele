#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>

using namespace std;

// Générateur aléatoire global — utilisé AVANT les threads donc thread-safe ici
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 100);

// ─────────────────────────────────────────────────────────────────────────────
// Version A : distribution ALTERNÉE des lignes de C entre les threads
// Exemple (4 threads, 5 lignes) :
//   ligne 0 → T0 | ligne 1 → T1 | ligne 2 → T2 | ligne 3 → T3 | ligne 4 → T0
// ─────────────────────────────────────────────────────────────────────────────
void worker_alternated(const int* A, const int* B, int* C,
                       int N, int threadId, int nbThreads)
{
    for (int i = threadId; i < N; i += nbThreads) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Version B : distribution CONTIGUË des lignes de C entre les threads
// Exemple (4 threads, 8 lignes) :
//   T0 → lignes 0-1 | T1 → lignes 2-3 | T2 → lignes 4-5 | T3 → lignes 6-7
// ─────────────────────────────────────────────────────────────────────────────
void worker_contiguous(const int* A, const int* B, int* C,
                       int N, int threadId, int nbThreads)
{
    int baseChunk = N / nbThreads;
    int lineStart = threadId * baseChunk;
    // Le dernier thread prend les lignes restantes
    int lineEnd = (threadId == nbThreads - 1) ? N : lineStart + baseChunk;

    for (int i = lineStart; i < lineEnd; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

int main() {

    int N = 0;
    cout << "Entrez la taille N : ";
    cin >> N;
    if (N <= 0) {
        cerr << "N doit etre un entier strictement positif." << endl;
        return 1;
    }

    int* A = new int[N * N];
    int* B = new int[N * N];
    int* C = new int[N * N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }

    // afficher le nombre de threads logiques du CPU
    unsigned int T = thread::hardware_concurrency();
    cout << "Threads logiques detectes sur ce CPU : " << T << endl;
    cout << "Valeurs suggerees par le cours : "
         << T / 4 << ", " << T / 2 << ", "
         << T     << ", " << T * 2 << " threads" << endl;

    // Demander le nombre de threads à utiliser
    int nbThreads = 0;
    cout << "Entrez le nombre de threads a utiliser : ";
    cin >> nbThreads;

    //Choisir le mode de distrib
    int mode = 0;
    cout << "Mode de distribution :" << endl;
    cout << "  0 = alternee  (T0 T1 T2 T3 T0 T1 ...)" << endl;
    cout << "  1 = contigue  (T0 T0 T1 T1 T2 T2 ...)" << endl;
    cout << "Votre choix : ";
    cin >> mode;

    //Snapshot CPU avant le calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Mesure du temps — uniquement le calcul
    auto start = chrono::steady_clock::now();

    // Création des threads
    vector<thread> threads;
    threads.reserve(nbThreads);

    for (int tid = 0; tid < nbThreads; tid++) {
        if (mode == 0) {
            threads.emplace_back(worker_alternated, A, B, C, N, tid, nbThreads);
        } else {
            threads.emplace_back(worker_contiguous, A, B, C, N, tid, nbThreads);
        }
    }

    // Attente de la fin de tous les threads
    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> duree_ms = end - start;
    chrono::duration<double>        duree_s  = end - start;

    //Snapshot CPU après le calcul
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

    // Affichage des résultats
    cout << "Duree du calcul C = A * B : " << duree_s.count()  << " s"  << endl;
    cout << "CPU utilise : " << fixed << setprecision(2) << cpu_percent << " %" << endl;
    cout.unsetf(ios::fixed);
    cout << "Memoire max : " << mem_mb << " Mo" << endl;

    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}