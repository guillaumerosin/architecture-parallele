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
            id_bis = id + BLOCK * THREADS * iterator;
        }
    }
}


int main()
{
    
}




