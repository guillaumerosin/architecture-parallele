#include "cuda_runtime.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

#define BLOCK 128
#define THREADS 128

#define CUDA_CHECK(call)                                                         \
    do                                                                           \
    {                                                                            \
        cudaError_t err__ = (call);                                              \
        if (err__ != cudaSuccess)                                                \
        {                                                                        \
            fprintf(stderr, "CUDA error %s:%d: %s\n", __FILE__, __LINE__,      \
                    cudaGetErrorString(err__));                                  \
            exit(EXIT_FAILURE);                                                  \
        }                                                                        \
    } while (0)

static void checkCudaRuntimeDriverCompatibility()
{
    int runtimeVersion = 0;
    int driverVersion = 0;

    CUDA_CHECK(cudaRuntimeGetVersion(&runtimeVersion));
    CUDA_CHECK(cudaDriverGetVersion(&driverVersion));

    if (driverVersion < runtimeVersion)
    {
        fprintf(stderr,
                "Incompatibilite CUDA: driver=%d runtime=%d. "
                "Mets a jour le driver NVIDIA ou compile avec un toolkit CUDA <= driver.\n",
                driverVersion, runtimeVersion);
        exit(EXIT_FAILURE);
    }
}

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
    checkCudaRuntimeDriverCompatibility();

    int Width;
    int blocksCount = BLOCK;
    int threadsPerBlock = THREADS;

    srand(time(NULL));

    printf("Taille de la matrice ? ");
    scanf("%d", &Width);

    if (Width <= 0)
    {
        printf("Taille invalide.\n");
        return 1;
    }

    const size_t size = (size_t)Width * (size_t)Width * sizeof(int);

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

    CUDA_CHECK(cudaMalloc((void**)&d_matA, size));
    CUDA_CHECK(cudaMalloc((void**)&d_matB, size));
    CUDA_CHECK(cudaMalloc((void**)&d_matC, size));

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
    CUDA_CHECK(cudaMemcpy(d_matA, h_matA, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_matB, h_matB, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_matC, h_matC, size, cudaMemcpyHostToDevice));

    // Warm-up pour stabiliser la mesure
    matriceproductkernel<<<blocks, nThreadsPerBlocks>>>(d_matA, d_matB, d_matC, Width);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    auto kernelStart = std::chrono::system_clock::now();
    matriceproductkernel<<<blocks, nThreadsPerBlocks>>>(d_matA, d_matB, d_matC, Width);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    auto kernelStop = std::chrono::system_clock::now();

    double elapsedSeconds = std::chrono::duration<double>(kernelStop - kernelStart).count();
    printf("\nChrono kernel : %.9f s", elapsedSeconds);

    // Transfert GPU -> CPU
    CUDA_CHECK(cudaMemcpy(h_matC, d_matC, size, cudaMemcpyDeviceToHost));

    auto totalEnd = std::chrono::system_clock::now();
    double totalSeconds = std::chrono::duration<double>(totalEnd - totalStart).count();
    printf("\nChrono total avec transferts : %.9f s\n", totalSeconds);

    CUDA_CHECK(cudaFree(d_matA));
    CUDA_CHECK(cudaFree(d_matB));
    CUDA_CHECK(cudaFree(d_matC));

    free(h_matA);
    free(h_matB);
    free(h_matC);

    return 0;
}