#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

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
    int stride = blockDim.x * gridDim.x;

    if (id < Width * Width)
    {
        for (int id_bis = id; id_bis < Width * Width; id_bis += stride)
        {
            int sum = 0;
            for (int i = 0; i < Width; i++)
            {
                sum += d_matA[id_bis - id_bis % Width + i] *
                       d_matB[id_bis % Width + i * Width];
            }
            d_matC[id_bis] = sum;
        }
    }
}

int main()
{
    int Width;
    int blocksCount = BLOCK;
    int threadsPerBlock = THREADS;

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

    // Lancement du kernel
    printf("Nombre de blocs ? (0 = %d) ", BLOCK);
    scanf("%d", &blocksCount);
    if (blocksCount <= 0)
        blocksCount = BLOCK;

    printf("Threads par bloc ? (0 = %d) ", THREADS);
    scanf("%d", &threadsPerBlock);
    if (threadsPerBlock <= 0)
        threadsPerBlock = THREADS;

    dim3 blocks(blocksCount, 1, 1);
    dim3 nThreadsPerBlocks(threadsPerBlock, 1, 1);

    auto totalStart = std::chrono::system_clock::now();

    // Transferts CPU -> GPU
    cudaMemcpy(d_matA, h_matA, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matB, h_matB, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matC, h_matC, size, cudaMemcpyHostToDevice);

    // Warm-up pour stabiliser la mesure
    matriceproductkernel<<<blocks, nThreadsPerBlocks>>>(d_matA, d_matB, d_matC, Width);
    cudaDeviceSynchronize();

    auto kernelStart = std::chrono::system_clock::now();
    matriceproductkernel<<<blocks, nThreadsPerBlocks>>>(d_matA, d_matB, d_matC, Width);
    cudaDeviceSynchronize();
    auto kernelStop = std::chrono::system_clock::now();

    cudaError_t launchError = cudaGetLastError();
    if (launchError != cudaSuccess)
    {
        printf("\nErreur CUDA kernel: %s\n", cudaGetErrorString(launchError));
    }

    double elapsedSeconds = std::chrono::duration<double>(kernelStop - kernelStart).count();
    printf("\nChrono kernel : %.9f s", elapsedSeconds);

    // Transfert GPU -> CPU
    cudaMemcpy(h_matC, d_matC, size, cudaMemcpyDeviceToHost);

    auto totalEnd = std::chrono::system_clock::now();
    double totalSeconds = std::chrono::duration<double>(totalEnd - totalStart).count();
    printf("\nChrono total avec transferts : %.9f s\n", totalSeconds);

    cudaFree(d_matA);
    cudaFree(d_matB);
    cudaFree(d_matC);

    return 0;
}