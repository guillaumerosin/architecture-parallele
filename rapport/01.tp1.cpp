#include <iostream> //pour afficher du tete
#include <chrono>
#include <random>


using namespace std;

int main() {
    int N;
    cout << "Indique moi une valeur N:";
    cin >> N;

    std::mt19937 gen(std::random_device{}());              // générateur
    std::uniform_int_distribution<int> dis(1, 10);       // distribution

    // Allocation de 3 matrices N * N
    int *A = new int[N * N];
    int *B = new int[N * N];
    int *C = new int[N * N];

    // Je parcours toutes mes cases de ma matrice N xN
    for(int i; i<N; j++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = dis(gen);
            B[i * N + j] = dis(gen);
        }
    }
}