#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <chrono>

using namespace std;

#define BLOCK 128
#define THREADS 128

void printMatrix(int *d_matC, int Width)
{
    for (int idxM = 0; idxM < Width; idxM++)
    {
        for (int idxN = 0; idxN < Width; idxN++)
        {
            printf("%i\t", d_matC[(idxM * Width) + idxN]);
        }
        printf("\n");
    }
    printf("\n");
}

__global__ void matriceproductkernel(int *d_matA, int *d_matB, int *d_matC, int Width)
{
    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if (id < Width * Width)
    {
        int iterator = 0;
        int id_bis = id;
        while (id_bis < Width * Width)
        {
            for (int i = 0; i < Width; i++)
            {
                d_matC[id_bis] += d_matA[id_bis - id_bis % Width + i] *
                                   d_matB[id_bis % Width + i * Width];
            }
            iterator++;
            id_bis = id + BLOCK * THREADS * iterator;
        }
    }
}

int main()
{
    int Width;

    srand(time(NULL));

    chrono::time_point<std::chrono::system_clock> start, end;

    printf("Taille de la matrice ? ");
    scanf("%d", &Width);

    const int size = Width * Width * sizeof(int);

    int *h_matA, *h_matB, *h_matC;

    h_matA = (int*)malloc(size);
    h_matB = (int*)malloc(size);
    h_matC = (int*)malloc(size);

    // Initialisation aléatoire de la matrice A
    for (int i = 0; i < Width; ++i)
        for (int j = 0; j < Width; ++j)
            h_matA[i * Width + j] = rand() % 10;

    // Initialisation aléatoire de la matrice B
    for (int i = 0; i < Width; ++i)
        for (int j = 0; j < Width; ++j)
            h_matB[i * Width + j] = rand() % 10;

    // Initialisation de la matrice C à 0
    for (int i = 0; i < Width; i++)
        for (int j = 0; j < Width; j++)
            h_matC[i * Width + j] = 0;

    // Allocation mémoire sur le device
    int *d_matA, *d_matB, *d_matC;

    cudaMalloc((void**)&d_matA, size);
    cudaMalloc((void**)&d_matB, size);
    cudaMalloc((void**)&d_matC, size);

    // Copie des matrices A et B vers le device
    cudaMemcpy(d_matA, h_matA, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matB, h_matB, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matC, h_matC, size, cudaMemcpyHostToDevice);

    // Lancement du kernel
    dim3 blocks(BLOCK, 1, 1);
    dim3 nThreadsPerBlocks(THREADS, 1, 1);

    start = std::chrono::system_clock::now();

    matriceproductkernel<<<blocks, nThreadsPerBlocks>>>(d_matA, d_matB, d_matC, Width);

    cudaDeviceSynchronize();

    end = std::chrono::system_clock::now();
    double microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    printf("\nChrono : %f", microseconds / 1000000.0);

    cudaMemcpy(h_matC, d_matC, size, cudaMemcpyDeviceToHost);

    cudaFree(d_matA);
    cudaFree(d_matB);
    cudaFree(d_matC);

    return 0;
}