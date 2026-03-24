//TP latch/barrier - Rosin Guillaume 
//la fonction rand() n'est pas thread-safe

#include <cstdio>
#include <random>
#include <thread>
#include <latch>
#include <barrier>
#include <vector>
#include <atomic>

#define N 5

std::latch latch(N); //j'initialise le compteur à N

int tab[N];
std::barrier bar(N);
std::atomic<int> active{N};

void thread_fct(int id){
    //de la sorte chacun de mes thread a son propre générateur, initialisé aléatoirement
    thread_local std::mt19937 random{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 20);
    
    while (true) {
        int current = active.load();
        if (id >= current) {
            return;
        }

        //Le thread génère un nombre et le stocke dans sa case
        tab[id] = dist(random);
        printf("tab[%d]=%d\n", id, tab[id]);

        // je souhaite attendre que tout le monde a écrit
        bar.arrive_and_wait();

        if (id==0){  //on calcule ma moyenne
            float moy = 0;
            for (int j = 0; j < current; ++j) moy += tab[j];
            moy /= current;
            printf("moy = %f\n", moy);
        }

        //re-synchronisation avant affichage pause
        bar.arrive_and_wait();

        if (current == 1) {
            return;
        }

        if (id == current - 1) {
            printf("thread[%d] je me casse\n", id);
            active.fetch_sub(1);
        }

        // tout le monde attend que le message de sortie soit affiché
        bar.arrive_and_wait();

        if (id == current - 1) {
            bar.arrive_and_drop();
            return;
        }
    }
}

int main(){
    std::vector<std::thread> threads;
    for (int i=0;i<N;++i) threads.emplace_back(thread_fct, i);

    for (int i=0;i<N;++i) threads[i].join();
   
    return 0;

}