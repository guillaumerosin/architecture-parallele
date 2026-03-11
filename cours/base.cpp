#include <cstdio>
#include <iostream>
#include <vector>


int main() {

    int N;
    std::cout << "Il était une fois, un monde stable... équilibré...\n";
    std::cout << "Un monde sans Bryan  \n";
    std::cout << "Entre une valeur entiere N pour que je définisse la taille de la matrice ma loute";
    std::cin >> N;
    std::cout << "N =" << N << std::endl;
    
    
    int* Matrice = new int[row * col];

    for (int i = 0; i < row ; i++) {
        for (int j = 0; j < col; j++) {
            Matrice[i * col +j] = i * cols +j    // accès à la case i,j de la matrice + ce qu'on y écrit 
        }
        int x = Matrice[2 * cols +3]
        std::cout << x << std::endl;
        delete[] Matrice;
        return 0;
    }
}
