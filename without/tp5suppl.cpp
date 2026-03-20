#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cuda_runtime.h>

using namespace std;

// Kernel CUDA : chaque thread calcule un élément C[i][j] = A[i] · BT[j]
__global__ void calcul(int* A, int* BT, int* C, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < N * N) {
        int row = idx / N;  // ligne de A
        int col = idx % N;  // colonne de B (ligne de BT)

        int somme = 0;
        for (int k = 0; k < N; k++) {
            somme += A[row * N + k] * BT[col * N + k];
        }
        C[idx] = somme;
    }
}

int main() {
    int N          = 0;
    int nbr_thread = 256;

    cout << "Un nombre entier ? : ";
    cin >> N;

    int size = N * N * sizeof(int);

    // Allocation des matrices (format linéaire pour CUDA)
    int*  A        = new int[N * N];
    int*  C        = new int[N * N];
    int*  B_transpo = new int[N * N];
    int** B        = new int*[N];

    srand(time(NULL));

    // Initialisation de A
    for (int i = 0; i < N * N; i++) {
        A[i] = rand() % 10 + 1;
    }

    // Initialisation de B et calcul de sa transposée
    for (int i = 0; i < N; i++) {
        B[i] = new int[N];
        for (int j = 0; j < N; j++) {
            B[i][j]             = rand() % 10 + 1;
            B_transpo[j * N + i] = B[i][j];
        }
    }

    // --- Début mesure temps total (alloc GPU + calcul + copie) ---
    chrono::time_point<chrono::system_clock> start, end;
    start = chrono::system_clock::now();

    // Allocation et transfert vers le GPU
    int* MGA, *MGB, *MGC;
    cudaMalloc((void**)&MGA, size);
    cudaMalloc((void**)&MGB, size);
    cudaMalloc((void**)&MGC, size);

    cudaMemcpy(MGA, A,        size, cudaMemcpyHostToDevice);
    cudaMemcpy(MGB, B_transpo, size, cudaMemcpyHostToDevice);

    // Configuration du kernel
    dim3 threadsPerBlock(nbr_thread);
    dim3 numBlocks((N * N + threadsPerBlock.x - 1) / threadsPerBlock.x);

    // --- Début mesure temps calcul seul ---
    chrono::time_point<chrono::system_clock> start2, end2;
    start2 = chrono::system_clock::now();

    calcul<<<numBlocks, threadsPerBlock>>>(MGA, MGB, MGC, N);
    cudaDeviceSynchronize();

    cudaMemcpy(C, MGC, size, cudaMemcpyDeviceToHost);

    end2 = chrono::system_clock::now();
    // --- Fin mesure temps calcul seul ---

    end = chrono::system_clock::now();
    // --- Fin mesure temps total ---

    long long microseconds  = chrono::duration_cast<chrono::microseconds>(end  - start ).count();
    long long microseconds2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();

    cout << "Temps d'execution total  : " << microseconds  << " micro sec\n";
    cout << "Temps d'execution calcul : " << microseconds2 << " micro sec\n";

    // Libération mémoire GPU
    cudaFree(MGA);
    cudaFree(MGB);
    cudaFree(MGC);

    // Libération mémoire CPU
    for (int i = 0; i < N; i++) {
        delete[] B[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;
    delete[] B_transpo;

    return 0;
}