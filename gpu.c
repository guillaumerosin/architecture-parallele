// Crée dynamiquement un tableau à 2D en C++

#include <stdio.h>
#include <stdlib.h>

int **ary;   //Double pointeur
ary = new int*[sizeY];  //Allocation d'un vecteur de pointeurs

for (int i = 0; i < sizeY; ++i) {
    ary[i] = new int[sizeX];
}

# pour désallouer le tableau
for(int i = 0; i < sizeY; ++i) {
    delete[] ary[i];
}
delete[] ary;


// malloc et new en C
// new et delete en C++