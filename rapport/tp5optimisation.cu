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
        int row = id / N; // ligne de A
        int col = id % N; // ligne de BT (= colonne de B)

        int sum = 0;
        for (int k = 0; k < N; ++k)
        {
            // A[row][k] * BT[col][k] => les deux accès sont séquentiels (coalescés)
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

    int nBlocks, threadsPerBlock;
    std::cout << "Nombre de blocs       : ";
    std::cin >> nBlocks;
    std::cout << "Threads par bloc      : ";
    std::cin >> threadsPerBlock;

    auto start_total = std::chrono::steady_clock::now();

        cudaMemcpy(Agpu, A, size, cudaMemcpyHostToDevice);
        cudaMemcpy(Bgpu, B, size, cudaMemcpyHostToDevice);

        // --- Début mesure compute (BT inclus comme demandé) ---
        auto start_compute = std::chrono::steady_clock::now();

        int *BTgpu;
        cudaMalloc((void**)&BTgpu, size);                                        // allocation BT

        transposeKernel<<<nBlocks, threadsPerBlock>>>(Bgpu, BTgpu, N);           // calcul BT
        cudaDeviceSynchronize();

        matMulKernel<<<nBlocks, threadsPerBlock>>>(Agpu, BTgpu, Cgpu, N);        // produit avec BT
        cudaDeviceSynchronize();

        cudaFree(BTgpu);                                                          // suppression BT

        auto end_compute = std::chrono::steady_clock::now();
        // --- Fin mesure compute ---

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