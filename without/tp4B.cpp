#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <thread>
#include <sys/resource.h>

using namespace std;

// Calcul d'un bloc de lignes de la multiplication A x B_transposée
void calcul(int N, float** A, float** BT, float** C, int start_idx, int end_idx) {
    for (int i = start_idx; i < end_idx; ++i) {
        for (int j = 0; j < N; ++j) {
            float somme = 0.0f;
            for (int k = 0; k < N; ++k) {
                somme += A[i][k] * BT[j][k];
            }
            C[i][j] = somme;
        }
    }
}

int main() {
    int N          = 0;
    int nbr_thread = 0;

    cout << "Un nombre entier ? : ";
    cin >> N;
    cout << "Un nombre de threads ? : ";
    cin >> nbr_thread;

    // Allocation des matrices
    float** A        = new float*[N];
    float** B        = new float*[N];
    float** C        = new float*[N];
    float** B_transpo = new float*[N];

    srand(time(NULL));

    // Initialisation de A, B et C
    for (int i = 0; i < N; i++) {
        A[i] = new float[N];
        B[i] = new float[N];
        C[i] = new float[N];
        for (int j = 0; j < N; j++) {
            A[i][j] = static_cast<float>(rand() % 10 + 1);
            B[i][j] = static_cast<float>(rand() % 10 + 1);
            C[i][j] = 0.0f;
        }
    }

    // Calcul de la transposée de B
    for (int i = 0; i < N; i++) {
        B_transpo[i] = new float[N];
        for (int j = 0; j < N; j++) {
            B_transpo[i][j] = B[j][i];
        }
    }

    //ajout de la mesure pour le % du CPU 
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Je crée un chrono pour mesurer le temps entre start and end
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();


    thread* threads  = new thread[nbr_thread];
    int     chunkSize = N / nbr_thread;

    for (int i = 0; i < nbr_thread; i++) {
        int start_idx = i * chunkSize;
        int end_idx   = (i == nbr_thread - 1) ? N : (i + 1) * chunkSize;
        threads[i]    = thread(calcul, N, A, B_transpo, C, start_idx, end_idx);
    }

    // Attente de la fin de tous les threads
    for (int i = 0; i < nbr_thread; i++) {
        threads[i].join();
    }

    
        // je recupere l'instant actuel (instant ou le calcul est fini)
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        //Voir la durée en seconde
        std::chrono::duration<double> duree_s = end - start;
    
        //snapshot CPU après le calcul (pour voir le cpu usage en % )
    
        rusage usage_after{};
        getrusage(RUSAGE_SELF, &usage_after);
    
        double cpu_before =
            usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
            usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;
    
        double cpu_after =
            usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
            usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;
    
        double cpu_time = cpu_after - cpu_before;
        double cpu_percent = (cpu_time / duree_s.count()) * 100.0;
    
        // voir la mémoire utilisé een Mo
        double mem_mb = usage_after.ru_maxrss / 1024.0;
    
        cout << "Durée pour le calcul  ==> C = A * B : " << duree_s.count() << " s" << endl;
        cout << "CPU utilise : " << std::fixed << std::setprecision(2) << cpu_percent << " %" << std::endl;
        cout.unsetf(std::ios::fixed);
        cout << "Memoire utilisée : " << mem_mb << " Mo" << std::endl;
    


    // Libération de la mémoire
    for (int i = 0; i < N; i++) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
        delete[] B_transpo[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;
    delete[] B_transpo;
    delete[] threads;

    return 0;
}