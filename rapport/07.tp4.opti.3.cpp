#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>
#include <immintrin.h>   // _mm_malloc / _mm_free / AVX

using namespace std;

// Générateur aléatoire global — utilisé AVANT les threads donc thread-safe ici
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(1.0f, 100.0f);

// ─────────────────────────────────────────────────────────────────────────────
// Version A : distribution ALTERNÉE des lignes de C entre les threads
// Optimisation 1 : BT (transposée de B) → lecture séquentielle en mémoire
// Optimisation 2 : float → le CPU utilise l'instruction FMAD des unités AVX
// Optimisation 3 : données alignées sur 64 octets → AVX charge 64 bytes/cycle
// ─────────────────────────────────────────────────────────────────────────────
void worker_alternated(const float* A, const float* BT, float* C,
                       int N, int threadId, int nbThreads)
{
    for (int i = threadId; i < N; i += nbThreads) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                // FMAD : Fused Multiply-Add
                // A[i][k] * BT[j][k] + sum — une seule instruction AVX
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Version B : distribution CONTIGUË des lignes de C entre les threads
// Mêmes optimisations 1 + 2 + 3
// ─────────────────────────────────────────────────────────────────────────────
void worker_contiguous(const float* A, const float* BT, float* C,
                       int N, int threadId, int nbThreads)
{
    int baseChunk = N / nbThreads;
    int lineStart = threadId * baseChunk;
    int lineEnd   = (threadId == nbThreads - 1) ? N : lineStart + baseChunk;

    for (int i = lineStart; i < lineEnd; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
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

    // ─────────────────────────────────────────────────────────────────────────
    // 2. Allocation alignée sur 64 octets — Optimisation 3
    // _mm_malloc(taille_en_bytes, alignement)
    // On alloue N*N floats, chaque float = 4 bytes → N*N*sizeof(float) bytes
    // Alignement 64 octets = taille d'une ligne de cache CPU
    // ─────────────────────────────────────────────────────────────────────────
    float* A  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* B  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* BT = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* C  = (float*)_mm_malloc(N * N * sizeof(float), 64);

    if (!A || !B || !BT || !C) {
        cerr << "Erreur d'allocation memoire." << endl;
        return 1;
    }

    // 3. Remplir A et B avec des valeurs aléatoires entre 1.0 et 100.0
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
            C[i * N + j] = 0.0f;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // 4. Calculer la transposée de B — Optimisation 1
    // AVANT le chrono : ce n'est pas du calcul C = A * B
    // BT[j][k] = B[k][j]  →  BT[j*N+k] = B[k*N+j]
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
        _mm_free(A); _mm_free(B); _mm_free(BT); _mm_free(C);
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

    // ─────────────────────────────────────────────────────────────────────────
    // 12. Désallocation — _mm_free à la place de delete[] car _mm_malloc
    // ─────────────────────────────────────────────────────────────────────────
    _mm_free(A);
    _mm_free(B);
    _mm_free(BT);
    _mm_free(C);

    return 0;
}#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>
#include <immintrin.h>   // _mm_malloc / _mm_free / AVX

using namespace std;

// Générateur aléatoire global — utilisé AVANT les threads donc thread-safe ici
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(1.0f, 100.0f);

// ─────────────────────────────────────────────────────────────────────────────
// Version A : distribution ALTERNÉE des lignes de C entre les threads
// Optimisation 1 : BT (transposée de B) → lecture séquentielle en mémoire
// Optimisation 2 : float → le CPU utilise l'instruction FMAD des unités AVX
// Optimisation 3 : données alignées sur 64 octets → AVX charge 64 bytes/cycle
// ─────────────────────────────────────────────────────────────────────────────
void worker_alternated(const float* A, const float* BT, float* C,
                       int N, int threadId, int nbThreads)
{
    for (int i = threadId; i < N; i += nbThreads) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                // FMAD : Fused Multiply-Add
                // A[i][k] * BT[j][k] + sum — une seule instruction AVX
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Version B : distribution CONTIGUË des lignes de C entre les threads
// Mêmes optimisations 1 + 2 + 3
// ─────────────────────────────────────────────────────────────────────────────
void worker_contiguous(const float* A, const float* BT, float* C,
                       int N, int threadId, int nbThreads)
{
    int baseChunk = N / nbThreads;
    int lineStart = threadId * baseChunk;
    int lineEnd   = (threadId == nbThreads - 1) ? N : lineStart + baseChunk;

    for (int i = lineStart; i < lineEnd; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
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

    // ─────────────────────────────────────────────────────────────────────────
    // 2. Allocation alignée sur 64 octets — Optimisation 3
    // _mm_malloc(taille_en_bytes, alignement)
    // On alloue N*N floats, chaque float = 4 bytes → N*N*sizeof(float) bytes
    // Alignement 64 octets = taille d'une ligne de cache CPU
    // ─────────────────────────────────────────────────────────────────────────
    float* A  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* B  = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* BT = (float*)_mm_malloc(N * N * sizeof(float), 64);
    float* C  = (float*)_mm_malloc(N * N * sizeof(float), 64);

    if (!A || !B || !BT || !C) {
        cerr << "Erreur d'allocation memoire." << endl;
        return 1;
    }

    // 3. Remplir A et B avec des valeurs aléatoires entre 1.0 et 100.0
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
            C[i * N + j] = 0.0f;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // 4. Calculer la transposée de B — Optimisation 1
    // AVANT le chrono : ce n'est pas du calcul C = A * B
    // BT[j][k] = B[k][j]  →  BT[j*N+k] = B[k*N+j]
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
        _mm_free(A); _mm_free(B); _mm_free(BT); _mm_free(C);
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

    // ─────────────────────────────────────────────────────────────────────────
    // 12. Désallocation — _mm_free à la place de delete[] car _mm_malloc
    // ─────────────────────────────────────────────────────────────────────────
    _mm_free(A);
    _mm_free(B);
    _mm_free(BT);
    _mm_free(C);

    return 0;
}