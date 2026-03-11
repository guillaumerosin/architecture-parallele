// Dans ce code chaque ligne est éparpillé dans la mémoire,
// le cpu doit sauter partout pour lire la data donc c'est encore plus lent.

//tandis que dans la nouvelle version, tout est collé en mémoire,
// le cpu peut tout lire d'un coup grâce au cache = plus rapide.

// RealTimeSys_Exo1.cpp — Rosin Guillaume, TP1 - Calcul séquentiel
// Utilise le style C original (malloc/free, printf/scanf) avec les corrections

#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <sys/resource.h>
#include <iomanip>

using namespace std;

void produire_ma_matrice(int **matA, int **matB, int **matC, int N);
void matriceproduct(int **matA, int **matB, int **matC, int size);

static mt19937 gen(random_device{}());
static uniform_int_distribution<> dis(1, 100);

int main() {
    while (true) {
        int size;

        cout << "Donne moi la taille de la matrice biloute :";
        if (!(cin >> size) || size <= 0) {
            cout << "Tu dois indiquer un nombre entier positif pour la taille de la matrice biloute"  << endl;
            break;
        }

        int **matA = new int *[size];  
        int **matB = new int *[size];
        int **matC = new int *[size];

        for (int i = 0; i < size; i++)
        {
            matA[i] = new int[size];
            matB[i] = new int[size];
            matC[i] = new int[size](); // () initialise à 0
        }

        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
            {
                matA[i][j] = dis(gen);
                matB[i][j] = dis(gen);
            }
        
            cout << "***********************************************" << endl;

            // Prendre un snapshot de l'utilisation CPU avant le calcul
            struct rusage usage_before{};
            getrusage(RUSAGE_SELF, &usage_before);

            chrono::steady_clock::time_point start = chrono::steady_clock::now();
            matriceproduct(matA, matB, matC, size);
            chrono::steady_clock::time_point end = chrono::steady_clock::now();

            chrono::duration<double, milli> duree_ms = end - start;
            chrono::duration<double> duree_s = end - start;

            // Snapshot après le calcul
            struct rusage usage_after{};
            getrusage(RUSAGE_SELF, &usage_after);

            // Temps CPU user + système AVANT
            double cpu_before =
                usage_before.ru_utime.tv_sec + usage_before.ru_utime.tv_usec / 1e6 +
                usage_before.ru_stime.tv_sec + usage_before.ru_stime.tv_usec / 1e6;

            // Temps CPU user + système APRÈS
            double cpu_after =
                usage_after.ru_utime.tv_sec + usage_after.ru_utime.tv_usec / 1e6 +
                usage_after.ru_stime.tv_sec + usage_after.ru_stime.tv_usec / 1e6;

            // Temps CPU consommé uniquement pendant le produit de matrices
            double cpu_time = cpu_after - cpu_before;

            // Pourcentage d'utilisation CPU sur l'intervalle [start, end]
            double cpu_percent = (cpu_time / duree_s.count()) * 100.0;

            double mem_mb = usage_after.ru_maxrss / 1024.0;

            cout << "Taille : " << size << " x " << size << endl;
            cout << "Durée calcul en mili seconde: " << duree_ms.count() << " ms" << endl;
            cout << "Durée calcul en seconde: " << duree_s.count() << " s" << endl;
            cout << "CPU utilise : " << fixed << setprecision(3) << cpu_percent << " %" << endl;
            cout.unsetf(ios::fixed);
            cout << "Memoire max : " << mem_mb << " Mo" << endl;
            cout << "-------------------------------------------------" << endl;

            for (int i = 0; i < size; i++)
        {
            delete[] matA[i];
            delete[] matB[i];
            delete[] matC[i];
        }
        delete[] matA;
        delete[] matB;
        delete[] matC;
    }

    return 0;
}

void matriceproduct(int **matA, int **matB, int **matC, int size)
{
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
        {
            int sum = 0;
            for (int k = 0; k < size; k++)
                sum += matA[i][k] * matB[k][j];
            matC[i][j] = sum;
        }
}

