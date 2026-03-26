#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// générateur aléatoire pour remplir A et B
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_d#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    int N = 0;
    cout << "Un nombre entier ? : ";
    cin >> N;

    // Allocation dynamique de trois matrices N x N
    int** A = new int*[N];
    int** B = new int*[N];
    int** C = new int*[N];

    srand(time(NULL));

    // Initialisation des matrices A et B avec des valeurs aléatoires
    for (int i = 0; i < N; i++) {
        A[i] = new int[N];
        B[i] = new int[N];
        C[i] = new int[N];

        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10 + 1; // Valeur entre 1 et 10
            B[i][j] = rand() % 10 + 1;
            C[i][j] = 0;
        }
    }

    // Mesure du temps de début
    chrono::time_point<ch#include <chrono>
    #include <cstdio>
    #include <iostream>
    #include <random>
    
    #include <thread>
    #include <vector>
    
    // générateur aléatoire pour remplir A et B
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_d#include <iostream>
    #include <chrono>
    #include <cstdlib>
    #include <ctime>
    
    using namespace std;
    
    int main() {
        int N = 0;
        cout << "Un nombre entier ? : ";
        cin >> N;
    
        // Allocation dynamique de trois matrices N x N
        int** A = new int*[N];
        int** B = new int*[N];
        int** C = new int*[N];
    
        srand(time(NULL));
    
        // Initialisation des matrices A et B avec des valeurs aléatoires
        for (int i = 0; i < N; i++) {
            A[i] = new int[N];
            B[i] = new int[N];
            C[i] = new int[N];
    
            for (int j = 0; j < N; j++) {
                A[i][j] = rand() % 10 + 1; // Valeur entre 1 et 10
                B[i][j] = rand() % 10 + 1;
                C[i][j] = 0;
            }
        }
    
        // Mesure du temps de début
        chrono::time_point<chrono::system_clock> start, end;
        start = chrono::system_clock::now();
    
        // Multiplication matricielle : C = A x B
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int somme = 0;
                for (int k = 0; k < N; k++) {
                    somme += A[i][k] * B[k][j];
                }
                C[i][j] = somme;
            }
        }
    
        // Mesure du temps de fin
        end = chrono::system_clock::now();
        lon    // 7. désallouer les matrices
        g long int microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
        cout << "Temps d'execution : " << microseconds << " micro sec\n";
    
        // Libération de la mémoire
        for (int i = 0; i < N; i++) {
            delete[] A[i];
            delete[] B[i];
            delete[] C[i];
        }
        delete[] A;
        delete[] B;
        delete[] C;
    
        return 0;
    }
    istribution<> dis(1, 100); // valeurs entre 1 et 100
    
    // Travail effectué par un thread : calcule un sous-ensemble de cases de C
    void worker_range(const int* A, const int* B, int* C,
                      int N, int threadId, int nbThreads,
                      bool alternated)
    {
        const int totalCells = N * N;
    
        if (alternated) {
            // Version A : distribution alternée des cases de C
            for (int idx = threadId; idx < totalCells; idx += nbThreads) {
                int i = idx / N;
                int j = idx % N;
                int sum = 0;
                for (int k = 0; k < N; ++k) {
                    sum += A[i * N + k] * B[k * N + j];
                }
                C[i * N + j] = sum;
            }
        } else {
            // Version B : distribution contiguë des cases de C
            int baseChunk = totalCells / nbThreads;
            int start = threadId * baseChunk;
            int end = (threadId == nbThreads - 1) ? totalCells : start + baseChunk;
    
            for (int idx = start; idx < end; ++idx) {
                int i = idx / N;
                int j = idx % N;
                int sum = 0;
                for (int k = 0; k < N; ++k) {
                    sum += A[i * N + k] * B[k * N + j];
                }
                C[i * N + j] = sum;
            }
        }
    }
    
    int main(void) {
        using std::cout;
        using std::endl;
    
        int N;
        std::printf("Entre une valeur entier N:\n");
        if (std::scanf("%d", &N) != 1 || N <= 0) {
            std::fprintf(stderr, "N doit être un entier strictement positif.\n");
            return 1;
        }
    
        // 2. allouer dynamiquement A, B, C (N x N)
        int *A = new int[N * N];
        int *B = new int[N * N];
        int *C = new int[N * N];
    
        // 3. remplir A et B avec des valeurs aléatoires entre 1 et 100
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                A[i * N + j] = dis(gen);
                B[i * N + j] = dis(gen);
            }
        }
    
        // 4. demander le nombre de threads à utiliser
        unsigned int hwThreads = std::thread::hardware_concurrency();
        if (hwThreads > 0) {
            cout << "Threads logiques detectes sur le CPU : " << hwThreads << endl;
        }
    
        int nbThreads;
        cout << "Entrez le nombre de threads a utiliser pour le calcul : ";
        std::cin >> nbThreads;
        if (nbThreads <= 0) {
            std::fprintf(stderr, "Nombre de threads invalide.\n");
            delete[] A;
            delete[] B;
            delete[] C;
            return 1;
        }
    
        const int totalCells = N * N;
        if (nbThreads > totalCells) {
            nbThreads = totalCells; // inutile d'avoir plus de threads que de cases
        }
    
        // choisir la distribution : true = alternée, false = contiguë
        int mode = 0;
        cout << "Mode de distribution (0 = alternee, 1 = contigue) : ";
        std::cin >> mode;
        bool alternated = (mode == 0);
    
        // 5. mesurer uniquement le temps de calcul de C = A * B
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    
        std::vector<std::thread> threads;
        threads.reserve(nbThreads);
        for (int tid = 0; tid < nbThreads; ++tid) {
            threads.emplace_back(worker_range, A, B, C, N, tid, nbThreads, alternated);
        }
        for (auto& t : threads) {
            t.join();
        }
    
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duree_ms = end - start;
    
        // 6. afficher la durée du calcul uniquement
        cout << "Duree du calcul C = A * B : " << duree_ms.count() << " ms" << endl;
    
        // 7. désallouer les matrices
        delete[] A;
        delete[] B;
        delete[] C;
    
        return 0;
    }
    
    
    rono::system_clock> start, end;
    start = chrono::system_clock::now();

    // Multiplication matricielle : C = A x B
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int somme = 0;
            for (int k = 0; k < N; k++) {
                somme += A[i][k] * B[k][j];
            }
            C[i][j] = somme;
        }
    }

    // Mesure du temps de fin
    end = chrono::system_clock::now();
    lon    // 7. désallouer les matrices
    g long int microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Temps d'execution : " << microseconds << " micro sec\n";

    // Libération de la mémoire
    for (int i = 0; i < N; i++) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
istribution<> dis(1, 100); // valeurs entre 1 et 100

// Travail effectué par un thread : calcule un sous-ensemble de cases de C
void worker_range(const int* A, const int* B, int* C,
                  int N, int threadId, int nbThreads,
                  bool alternated)
{
    const int totalCells = N * N;

    if (alternated) {
        // Version A : distribution alternée des cases de C
        for (int idx = threadId; idx < totalCells; idx += nbThreads) {
            int i = idx / N;
            int j = idx % N;
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    } else {
        // Version B : distribution contiguë des cases de C
        int baseChunk = totalCells / nbThreads;
        int start = threadId * baseChunk;
        int end = (threadId == nbThreads - 1) ? totalCells : start + baseChunk;

        for (int idx = start; idx < end; ++idx) {
            int i = idx / N;
            int j = idx % N;
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

int main(void) {
    using std::cout;
    using std::endl;

    int N;
    std::printf("Entre une valeur entier N:\n");
    if (std::scanf("%d", &N) != 1 || N <= 0) {
        std::fprintf(stderr, "N doit être un entier strictement positif.\n");
        return 1;
    }

    // 2. allouer dynamiquement A, B, C (N x N)
    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    // 3. remplir A et B avec des valeurs aléatoires entre 1 et 100
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }

    // 4. demander le nombre de threads à utiliser
    unsigned int hwThreads = std::thread::hardware_concurrency();
    if (hwThreads > 0) {
        cout << "Threads logiques detectes sur le CPU : " << hwThreads << endl;
    }

    int nbThreads;
    cout << "Entrez le nombre de threads a utiliser pour le calcul : ";
    std::cin >> nbThreads;
    if (nbThreads <= 0) {
        std::fprintf(stderr, "Nombre de threads invalide.\n");
        delete[] A;
        delete[] B;
        delete[] C;
        return 1;
    }

    const int totalCells = N * N;
    if (nbThreads > totalCells) {
        nbThreads = totalCells; // inutile d'avoir plus de threads que de cases
    }

    // choisir la distribution : true = alternée, false = contiguë
    int mode = 0;
    cout << "Mode de distribution (0 = alternee, 1 = contigue) : ";
    std::cin >> mode;
    bool alternated = (mode == 0);

    // 5. mesurer uniquement le temps de calcul de C = A * B
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(nbThreads);
    for (int tid = 0; tid < nbThreads; ++tid) {
        threads.emplace_back(worker_range, A, B, C, N, tid, nbThreads, alternated);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duree_ms = end - start;

    // 6. afficher la durée du calcul uniquement
    cout << "Duree du calcul C = A * B : " << duree_ms.count() << " ms" << endl;

    // 7. désallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}


