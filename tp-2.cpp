#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <vector>

void calcul_ma_case(const int* A, const int* B, int* C, int N, int i, int j) {
    int sum = 0;
    for (int k = 0; k < N; ++k) {
        sum += A[i * N + k] * B[k * N + j];
    }
    C[i * N + j] = sum;
}

