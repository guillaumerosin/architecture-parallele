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
// Optimisation 1 : BT (transposée de B) → lecture séquentielle en mémoire
//   au lieu de lire les colonnes de B (sauts mémoire)
//   on lit les lignes de BT (séquentiel) → moins de cache miss
// ─────────────────────────────────────────────────────────────────────────────
void worker_alternated(const int* A, const int* BT, int* C,
                       int N, int threadId, int nbThreads)
{
    for (int i = threadId; i < N; i += nbThreads) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                // Ligne i de A  ×  Ligne j de BT  (et non colonne j de B)
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Version B : distribution CONTIGUË des lignes de C entre les threads
// Même optimisation 1
// ─────────────────────────────────────────────────────────────────────────────
void worker_contiguous(const int* A, const int* BT, int* C,
                       int N, int threadId, int nbThreads)
{
    int baseChunk = N / nbThreads;
    int lineStart = threadId * baseChunk;
    int lineEnd   = (threadId == nbThreads - 1) ? N : lineStart + baseChunk;

    for (int i = lineStart; i < lineEnd; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

int main() {

    // 1. Demander N à l'utilisateur
    int N = 0;
    cout << "Entrez la taille N des matrices : ";
    cin >> N;
    if (N <= 0) {
        cerr << "N doit etre un entier strictement positif." << endl;
        return 1;
    }

    // 2. Allouer dynamiquement les 4 matrices A, B, BT, C de taille N*N
    int* A  = new int[N * N];
    int* B  = new int[N * N];
    int* BT = new int[N * N];   // transposée de B — Optimisation 1
    int* C  = new int[N * N];

    // 3. Remplir A et B avec des valeurs aléatoires entre 1 et 100
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
            C[i * N + j] = 0;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // 4. Calculer la transposée de B — Optimisation 1
    // AVANT le chrono : ce n'est pas du calcul C = A * B
    // BT[j][k] = B[k][j]  →  BT[j*N+k] = B[k*N+j]
    // La colonne j de B devient la ligne j de BT
    // ─────────────────────────────────────────────────────────────────────────
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            BT[i * N + j] = B[j * N + i];
        }
    }

    // 5. Afficher le nombre de threads logiques du CPU
    unsigned int T = thread::hardware_concurrency();
    cout << "Threads logiques detectes sur ce CPU : " << T << endl;
    cout << "Valeurs suggerees par le cours : "
         << T / 4 << ", " << T / 2 << ", "
         << T     << ", " << T * 2 << " threads" << endl;

    int nbThreads = 0;
    cout << "Entrez le nombre de threads a utiliser : ";
    cin >> nbThreads;
    if (nbThreads <= 0) {
        cerr << "Nombre de threads invalide." << endl;
        delete[] A; delete[] B; delete[] BT; delete[] C;
        return 1;
    }
    if (nbThreads > N) {
        cout << "Attention : nbThreads > N, ramene a " << N << endl;
        nbThreads = N;
    }

    // 6. Choisir le mode de distribution
    int mode = 0;
    cout << "Mode de distribution :" << endl;
    cout << "  0 = alternee  (T0 T1 T2 T3 T0 T1 ...)" << endl;
    cout << "  1 = contigue  (T0 T0 T1 T1 T2 T2 ...)" << endl;
    cout << "Votre choix : ";
    cin >> mode;

    // 7. Snapshot CPU avant le calcul
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // 8. Mesure du temps — uniquement le calcul C = A * B (slide 23)
    auto start = chrono::steady_clock::now();

    // 9. Création des threads (slide 26)
    vector<thread> threads;
    threads.reserve(nbThreads);

    for (int tid = 0; tid < nbThreads; tid++) {
        if (mode == 0) {
            threads.emplace_back(worker_alternated, A, BT, C, N, tid, nbThreads);
        } else {
            threads.emplace_back(worker_contiguous, A, BT, C, N, tid, nbThreads);
        }
    }

    // Attente de la fin de tous les threads (slide 26)
    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> duree_ms = end - start;
    chrono::duration<double>        duree_s  = end - start;

    // 10. Snapshot CPU après le calcul
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

    // 11. Affichage des résultats
    cout << "Duree du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    cout << "Duree du calcul C = A * B : " << duree_s.count()  << " s"  << endl;
    cout << "CPU utilise : " << fixed << setprecision(2) << cpu_percent << " %" << endl;
    cout.unsetf(ios::fixed);
    cout << "Memoire max : " << mem_mb << " Mo" << endl;

    // 12. Désallouer toutes les matrices (slide 19)
    delete[] A;
    delete[] B;
    delete[] BT;
    delete[] C;

    return 0;
}