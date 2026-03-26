#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
// TP Sémaphore — Slide 58
// - 3 threads "writers" qui écrivent en boucle 20 valeurs dans le buffer
// - 2 threads "readers" qui lisent en permanence les nouvelles valeurs
// - Buffer circulaire de taille N
// ─────────────────────────────────────────────────────────────────────────────

#define Nwriters 3
#define Nreaders 2
#define N 10

int buffer[N];

// Mutex pour protéger l'indice d'écriture (section critique — slide 33)
std::mutex mutex_writer;
int writing_indice = 0;

// Mutex pour protéger l'indice de lecture (section critique — slide 33)
std::mutex mutex_reader;
int reading_indice = 0;

// Sémaphore d'écriture : N cases disponibles au départ (slide 55-56)
std::counting_semaphore<N> sem_write(N);

// Sémaphore de lecture : 0 valeurs à lire au départ (slide 55-56)
std::counting_semaphore<N> sem_read(0);

// Mutex pour protéger l'affichage (évite les mélanges de cout)
std::mutex screen_mutex;

// Compteur atomique du nombre de cases écrites non encore lues (slide 43-44)
std::atomic<int> job2do = 0;

// Flag de fin — atomic pour être thread-safe (slide 43-44)
std::atomic<bool> end_flag = false;

// ─────────────────────────────────────────────────────────────────────────────
// Thread writer : écrit N*2 valeurs dans le buffer circulaire
// ─────────────────────────────────────────────────────────────────────────────
void writer(int id)
{
    for (int i = 0; i < N * 2; ++i)
    {
        // Attend qu'une case soit libre (bloquant — slide 55)
        sem_write.acquire();

        // Section critique : réservation de la position d'écriture
        mutex_writer.lock();
        int pos = writing_indice;
        writing_indice++;
        if (writing_indice == N) writing_indice = 0;
        mutex_writer.unlock();

        // Écriture dans le buffer
        int val = rand() % 64;
        buffer[pos] = val;
        job2do.fetch_add(1);

        // Affichage protégé
        screen_mutex.lock();
        std::cout << "Writer " << id << " : Buffer[" << pos << "]=" << val << std::endl;
        screen_mutex.unlock();

        // Signale qu'une nouvelle valeur est disponible (slide 55)
        sem_read.release();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Thread reader : lit en permanence les nouvelles valeurs écrites
// Se termine quand end_flag = true ET toutes les valeurs ont été lues
// ─────────────────────────────────────────────────────────────────────────────
void reader(int id)
{
    while (true)
    {
        if (sem_read.try_acquire())
        {
            // Section critique : réservation de la position de lecture
            mutex_reader.lock();
            int pos = reading_indice;
            reading_indice++;
            if (reading_indice == N) reading_indice = 0;
            mutex_reader.unlock();

            int val = buffer[pos];
            job2do.fetch_sub(1);

            // Affichage protégé
            screen_mutex.lock();
            std::cout << "Reader " << id << " : Buffer[" << pos << "]=" << val << std::endl;
            screen_mutex.unlock();

            // Libère une case du buffer (slide 55)
            sem_write.release();
        }
        else
        {
            // Pas de valeur à lire — vérifie si on doit se terminer
            // end_flag = true ET job2do = 0 → tous les writers ont fini
            // et toutes les valeurs ont été lues
            if (end_flag && job2do.load() == 0)
            {
                screen_mutex.lock();
                std::cout << "Reader " << id << " job done !" << std::endl;
                screen_mutex.unlock();
                return;
            }
        }
    }
}

int main()
{
    srand(time(NULL));

    // Création des writers (style cours — slide 26)
    std::vector<std::thread> writers;
    writers.reserve(Nwriters);
    for (int i = 0; i < Nwriters; ++i)
        writers.emplace_back(writer, i + 1);

    // Création des readers (style cours — slide 26)
    std::vector<std::thread> readers;
    readers.reserve(Nreaders);
    for (int i = 0; i < Nreaders; ++i)
        readers.emplace_back(reader, 11 + i);

    // Attendre que tous les writers aient terminé
    for (auto& w : writers)
        w.join();

    // Signaler aux readers que les writers ont fini
    end_flag = true;

    // Attendre que tous les readers aient terminé
    for (auto& r : readers)
        r.join();

    return 0;
}