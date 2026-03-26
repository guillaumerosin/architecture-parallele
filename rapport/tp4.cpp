#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// Version A : distribution ALTERNÉE des lignes de C entre les threads
// Exemple (4 threads, 5 lignes) :
//   ligne 0 → T0 | ligne 1 → T1 | ligne 2 → T2 | ligne 3 → T3 | ligne 4 → T0
// ─────────────────────────────────────────────────────────────────────────────
void worker_alternated(int* A, int* B, int* C, int N, int threadId, int nbThreads)
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
void worker_contiguous(int* A, int* B, int* C,
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

    // 1. Demander N à l'utilisateur
    int N = 0;
    cout << "Entrez la taille N des matrices : ";
    cin >> N;

    // 2. Allouer dynamiquement les 3 matrices A, B, C de taille N*N
    int* A = new int[N * N];
    int* B = new int[N * N];
    int* C = new int[N * N];

    // 3. Remplir A et B avec des valeurs simples
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = i + j + 1;
            B[i * N + j] = i - j + 1;
        }
    }

    // 4. Afficher le nombre de threads logiques du CPU et demander le nombre a utiliser
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
        delete[] A; delete[] B; delete[] C;
        return 1;
    }
    // Securite : pas plus de threads que de lignes
    if (nbThreads > N) {
        cout << "Attention : nbThreads > N, ramene a " << N << endl;
        nbThreads = N;
    }

    // 5. Choisir le mode de distribution
    int mode = 0;
    cout << "Mode de distribution :" << endl;
    cout << "  0 = alternee  (T0 T1 T2 T3 T0 T1 ...)" << endl;
    cout << "  1 = contigue  (T0 T0 T1 T1 T2 T2 ...)" << endl;
    cout << "Votre choix : ";
    cin >> mode;

    // ─────────────────────────────────────────────────────────────────────────
    // 6. Lancer le calcul C = A * B et mesurer uniquement sa duree (slide 23)
    // ─────────────────────────────────────────────────────────────────────────
    auto start = chrono::steady_clock::now();

    vector<thread> threads;
    threads.reserve(nbThreads);

    for (int tid = 0; tid < nbThreads; tid++) {
        if (mode == 0) {
            threads.emplace_back(worker_alternated, A, B, C, N, tid, nbThreads);
        } else {
            threads.emplace_back(worker_contiguous, A, B, C, N, tid, nbThreads);
        }
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::steady_clock::now();

    // 7. Afficher la duree du calcul
    auto duree_us = chrono::duration_cast<chrono::microseconds>(end - start).count();
    auto duree_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Duree du calcul : " << duree_us << " us" << endl;
    cout << "Duree du calcul : " << duree_ms << " ms" << endl;

    // 8. Desallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}