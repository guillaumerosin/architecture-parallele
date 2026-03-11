#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <thread>
#include <vector>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 100);

void worker_range(const int* A, const int* B, int* C,
                  int N, int threadId, int nbThreads,
                  bool alternated)
{
    const int totalCells = N * N;

    if (alternated) {
        for (int idx = threadId; idx < totalCells; idx += nbThreads) {
            int i = idx / N;
            int j = idx % N;
            int sum = 0;
            for (int k = 0; k < N; ++k)
                sum += A[i * N + k] * B[k * N + j];
            C[i * N + j] = sum;
        }
    } else {
        int baseChunk = totalCells / nbThreads;
        int start = threadId * baseChunk;
        int end = (threadId == nbThreads - 1) ? totalCells : start + baseChunk;

        for (int idx = start; idx < end; ++idx) {
            int i = idx / N;
            int j = idx % N;
            int sum = 0;
            for (int k = 0; k < N; ++k)
                sum += A[i * N + k] * B[k * N + j];
            C[i * N + j] = sum;
        }
    }
}

int main(void)
{
    using std::cout;
    using std::endl;

    int N;
    std::printf("Entre une valeur entier N:\n");
    if (std::scanf("%d", &N) != 1 || N <= 0) {
        std::fprintf(stderr, "N doit être un entier strictement positif.\n");
        return 1;
    }

    int* A = new int[N * N];
    int* B = new int[N * N];
    int* C = new int[N * N];

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }

    unsigned int hwThreads = std::thread::hardware_concurrency();
    if (hwThreads > 0)
        cout << "Threads logiques detectes sur le CPU : " << hwThreads << endl;

    int nbThreads;
    cout << "Entrez le nombre de threads a utiliser pour le calcul : ";
    std::cin >> nbThreads;
    if (nbThreads <= 0) {
        std::fprintf(stderr, "Nombre de threads invalide.\n");
        delete[] A; delete[] B; delete[] C;
        return 1;
    }

    const int totalCells = N * N;
    if (nbThreads > totalCells)
        nbThreads = totalCells;

    int mode = 0;
    cout << "Mode de distribution (0 = alternee, 1 = contigue) : ";
    std::cin >> mode;
    bool alternated = (mode == 0);

    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(nbThreads);
    for (int tid = 0; tid < nbThreads; ++tid)
        threads.emplace_back(worker_range, A, B, C, N, tid, nbThreads, alternated);
    for (auto& t : threads)
        t.join();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    rusage usage_after{};
    getrusage(RUSAGE_SELF, &usage_after);

    std::chrono::duration<double, std::milli> duree_ms = end - start;
    std::chrono::duration<double>             duree_s  = end - start;

    double cpu_before =
        usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
        usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

    double cpu_after =
        usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
        usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

    double cpu_percent = ((cpu_after - cpu_before) / duree_s.count()) * 100.0;
    double mem_mb      = usage_after.ru_maxrss / 1024.0;

    cout << "***********************************************" << endl;
    cout << "Taille          : " << N << " x " << N << endl;
    cout << "Threads         : " << nbThreads << endl;
    cout << "Duree           : " << duree_ms.count() << " ms  |  "
         << duree_s.count() << " s" << endl;
    cout << "CPU utilise     : " << std::fixed << std::setprecision(2)
         << cpu_percent << " %" << endl;
    cout.unsetf(std::ios::fixed);
    cout << "Memoire max     : " << mem_mb << " Mo" << endl;
    cout << "-------------------------------------------------" << endl;

    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}