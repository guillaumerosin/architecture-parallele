#include <thread>
#include <cstdio>
#include <iostream>

int main(void) {
    int N;

    std::cout << "entre une valeur N:\n";
    std::cin >> N;

    int *A = new int[N * N]; //je définis ma matrice
    int *B = new int[N * N];
    int *C = new int[N * N];
    
    for (int i = 0; i < N; i++) {   //je l'a rempli
        for (int j = 0; j < N; j++) {
            A[i * N + j]  = dis(gen)  //tableau qui représente ma matrice A + je genere un nombre aléatoire
            B[i * N + j]  = dis(gen)  //tableau B
        }
    }
    //ajout de la mesure pour le % du CPU 
    rusage usage_before{};
    getrusage(RUSAGE_SELF, &usage_before);

    // Je crée un chrono pour mesurer le temps entre start and end
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
            for (int k = 0; k<N; ++k) {
                sum += A[i * N + k] * B[k * N +j];
            }
            C[i * N + j] = sum;
        }
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

    / 6. désallouer les matrices
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;



    
}


