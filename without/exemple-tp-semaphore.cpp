#include <iostream>
#include <thread>
#include <semaphore>
#include <cstdlib>

// INFO: pour se protéger contre des buffer overflow suffit de travailer avec une .size 

#define NOMBRE 10

int tableau[NOMBRE];

//je représente le nombre de cases libres dans mon buffer
std::counting_semaphore<NOMBRE> semEcrire(NOMBRE);
std::counting_semaphore<NOMBRE> semLire(0);

void fprod()
{
    int cpt = 0, donnee = 0;i
    for (int i=0;i <NOMBRE * 2;i++)
    {
        donnee = rand() % 1024;
        std::printf("Ecriture[%d]=%d\n", i, donnee);

        semEcrire.acquire();
        tableau[cpt] = donnee;
        semLire.release();

        cpt++;
        if (cpt >=NOMBRE) cpt = 0;
    }
}
void fcons()
{
    int cpt = 0, donnee = 0;
    for (int i = 0;i<NOMBRE * 2;i++)
    {
        semLire.acquire();
        donnee = tableau[cpt];
        semEcrire.release();

        std::printf("\tlecture[%d]=%d\n", i, donnee);
        cpt++;
        if (cpt >=NOMBRE) cpt = 0;
    }
}
int main()
{
    std::thread tprod(fprod);
    std::thread tcons(fcons);

    tprod.join();
    tcons.join();

    return 0;
}