#include <iostream>
#include <chrono>
#include <stdio.h>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"

#define BLOCK 128
#define THREADS 128

using namespace std;

//Dans le GPU chaque thread doit savoir quelle case il doit calculer

__global__ void firstFunction(int *A, int *B, int *C, int Width){
    int id = blockIdx.x * blockDim.x + threadIdx.x; //identifiant du thread dans la grille
    int stride = blockDim.x * gridDim.x;
    if (id < Width * Width)
    {
        int iterator = 0;    //compteur de passages pour que le meme thread puisse faire plusieurs cases
        int id_bis = id;     //index de la case de C actuellement traitee par ce thread.
        while (id_bis < Width * Width)
        {
            for (int i = 0; i < Width; i++)
            {
                C[id_bis] += A[id_bis - id_bis % Width + i] *
                                   B[id_bis % Width + i * Width];
            }
            iterator++;
            id_bis = id + stride * iterator;
        }
    }
}


int main()
{
    int Width;
    int blocksCount = BLOCK;
    int threadsPerBlock = THREADS;

    printf("Taille de la matrice ? ");
    scanf("%d", &Width);

    printf("Nombre de blocs ? (0 = %d) ", BLOCK);
    scanf("%d", &blocksCount);
    if (blocksCount <= 0)
    {
        blocksCount = BLOCK;
    }

    printf("Threads par bloc ? (0 = %d) ", THREADS);
    scanf("%d", &threadsPerBlock);
    if (threadsPerBlock <= 0)
    {
        threadsPerBlock = THREADS;
    }

    const int elemCount = Width * Width;
    const size_t size = elemCount * sizeof(int);

    int *h_matA = (int *)malloc(size);
    int *h_matB = (int *)malloc(size);
    int *h_matC = (int *)malloc(size);

    srand((unsigned int)time(NULL));
    for (int i = 0; i < elemCount; ++i)
    {
        h_matA[i] = rand() % 10;
        h_matB[i] = rand() % 10;
        h_matC[i] = 0;
    }

    int *d_matA = nullptr;
    int *d_matB = nullptr;
    int *d_matC = nullptr;

    cudaMalloc((void **)&d_matA, size);
    cudaMalloc((void **)&d_matB, size);
    cudaMalloc((void **)&d_matC, size);

    cudaMemcpy(d_matA, h_matA, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matB, h_matB, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matC, h_matC, size, cudaMemcpyHostToDevice);

    auto start = chrono::system_clock::now();
    firstFunction<<<blocksCount, threadsPerBlock>>>(d_matA, d_matB, d_matC, Width);
    cudaDeviceSynchronize();
    auto end = chrono::system_clock::now();

    double seconds = chrono::duration<double>(end - start).count();
    printf("\nChrono kernel : %f s\n", seconds);

    cudaMemcpy(h_matC, d_matC, size, cudaMemcpyDeviceToHost);

    cudaFree(d_matA);
    cudaFree(d_matB);
    cudaFree(d_matC);
    free(h_matA);
    free(h_matB);
    free(h_matC);

    return 0;
}




