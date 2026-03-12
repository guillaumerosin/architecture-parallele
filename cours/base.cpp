#include <iostream>

int main() {
    int N;
    std::cout << "Il était une fois, un monde stable... équilibré...\n";
    std::cout << "Un monde sans Bryan  \n";
    std::cout << "Entre une valeur entiere N pour que je définisse la taille de la matrice ma loute";
    std::cin >> N;

    if (!std::cin || N <= 0) {
        std::cerr << "N doit etre un entier strictement positif.\n";
        return 1;
    }

    std::cout << "N = " << N << std::endl;

    const int rows = N;
    const int cols = N;

    int* Matrice = new int[rows * cols];

    // remplissage de la matrice avec une valeur simple : M[i,j] = i * cols + j
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            Matrice[i * cols + j] = i * cols + j;
        }
    }

    // exemple d'accès : element de la ligne 2, colonne 3 (si assez grand)
    if (rows > 2 && cols > 3) {
        int x = Matrice[2 * cols + 3];
        std::cout << "M[2,3] = " << x << std::endl;
    }

    // affichage complet de la matrice
    std::cout << "Matrice :" << std::endl;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << Matrice[i * cols + j] << " ";
        }
        std::cout << std::endl;
    }

    delete[] Matrice;
    return 0;
}
