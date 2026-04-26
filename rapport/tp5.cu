#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>

__global__ void transposeKernel(int *B, int *BT, int N)
{
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int idx = id; idx < N * N; idx += stride)
    {
        int row = idx / N;
        int col = idx % N;
        BT[col * N + row] = B[row * N + col];
    }
}

__global__ void matMulKernel(int *A, int *BT, int *C, int N)
{
    int id = blockIdx.x * blockDim.x + threadIdx.x;

    while (id < N * N)
    {
        int row = id / N;
        int col = id % N;

        int sum = 0;
        for (int k = 0; k < N; ++k)
        {
            sum += A[row * N + k] * BT[col * N + k];
        }
        C[id] = sum;

        id += blockDim.x * gridDim.x;
    }
}

int main(void)
{
    int N;
    std::cout << "Entre une taille de matrice: ";
    std::cin >> N;

    int size = N * N * sizeof(int);

    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 99);
    for (int i = 0; i < N * N; i++)
    {
        A[i] = dis(gen);
        B[i] = dis(gen);
    }

    int *Agpu, *Bgpu, *Cgpu;
    cudaMalloc((void**)&Agpu, size);
    cudaMalloc((void**)&Bgpu, size);
    cudaMalloc((void**)&Cgpu, size);

    // Saisie du nombre de blocs et threads
    int nBlocks, threadsPerBlock;
    std::cout << "Nombre de blocs       (0 = 128) : ";
    std::cin >> nBlocks;
    if (nBlocks <= 0) nBlocks = 128;

    std::cout << "Threads par bloc      (0 = 256) : ";
    std::cin >> threadsPerBlock;
    if (threadsPerBlock <= 0) threadsPerBlock = 256;

    auto start_total = std::chrono::steady_clock::now();

        cudaMemcpy(Agpu, A, size, cudaMemcpyHostToDevice);
        cudaMemcpy(Bgpu, B, size, cudaMemcpyHostToDevice);

        auto start_compute = std::chrono::steady_clock::now();

        int *BTgpu;
        cudaMalloc((void**)&BTgpu, size);

        transposeKernel<<<nBlocks, threadsPerBlock>>>(Bgpu, BTgpu, N);
        cudaDeviceSynchronize();

        matMulKernel<<<nBlocks, threadsPerBlock>>>(Agpu, BTgpu, Cgpu, N);
        cudaDeviceSynchronize();

        cudaFree(BTgpu);

        auto end_compute = std::chrono::steady_clock::now();

        cudaMemcpy(C, Cgpu, size, cudaMemcpyDeviceToHost);

    auto end_total = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::milli> duree_compute = end_compute - start_compute;
    std::chrono::duration<double, std::milli> duree_total   = end_total   - start_total;

    std::cout << "Temps calcul GPU (BT inclus, SANS transferts A/B/C) : "
              << duree_compute.count() << " ms" << std::endl;
    std::cout << "Temps total      (AVEC transferts A/B/C)            : "
              << duree_total.count()   << " ms" << std::endl;

    cudaFree(Agpu);
    cudaFree(Bgpu);
    cudaFree(Cgpu);
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}