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
    
}


int main()
{
    
}




