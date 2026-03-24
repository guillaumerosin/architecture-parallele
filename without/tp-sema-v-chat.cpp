// TP Sémaphore : 3 writers, 2 readers, buffer circulaire
#include <iostream>
#include <thread>
#include <semaphore>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdlib>
#include <chrono>

// Taille du buffer circulaire
constexpr int BUFFER_SIZE = 10;

// Nombre de threads writers et readers
constexpr int NB_WRITERS = 3;
constexpr int NB_READERS = 2;

// Chaque writer écrit au minimum 20 valeurs
constexpr int VALUES_PER_WRITER = 20;
constexpr int TOTAL_VALUES = NB_WRITERS * VALUES_PER_WRITER;

// Buffer circulaire et index de lecture / écriture
int buffer[BUFFER_SIZE];
int write_pos = 0;
int read_pos = 0;

// Sémaphores :
std::counting_semaphore<BUFFER_SIZE> semEmpty(BUFFER_SIZE);  // - semEmpty : unitialise le nombre de cases libres
std::counting_semaphore<BUFFER_SIZE> semFull(0);  // - semFull  : nombre de cases remplies

// Mutex pour protéger les accès concurrents aux index/buffer
std::mutex write_mutex; //permet de protéger l'écriture dans mon buffer 
std::mutex read_mutex; //permet de protéger la lecture de mon buffer

// Nombre de cases non encore lues (me permet de compter combien de valeurs
//produites n'ont pas encore été lues dans le buffer)
std::atomic<int> unread_count{0};

// Indique que tous les writers ont terminé
std::atomic<bool> writers_done{false};

void writer(int id) {   //les threads writers
    for (int n = 0; n < VALUES_PER_WRITER; ++n) {
        // Simuler une production de donnée
        int value = id * 1000 + n; // juste pour identifier qui a écrit quoi

        // Attendre une case libre dans le buffer
        semEmpty.acquire();

        {
            std::lock_guard<std::mutex> lock(write_mutex);
            buffer[write_pos] = value;
            write_pos = (write_pos + 1) % BUFFER_SIZE;
            std::cout << "Writer " << id << " -> " << value << std::endl;
        }

        // Une nouvelle valeur est disponible à la lecture
        unread_count.fetch_add(1, std::memory_order_relaxed);
        semFull.release();
    }
}

void reader(int id) {
    int local_read = 0;

    while (true) {
        // Condition de sortie : plus rien à lire et tous les writers ont fini
        if (writers_done.load(std::memory_order_acquire) &&
            unread_count.load(std::memory_order_relaxed) == 0) {
            break;
        }

        // Essayer de prendre une valeur à lire
        if (!semFull.try_acquire()) {
            // Rien à lire pour le moment
            if (writers_done.load(std::memory_order_acquire) &&
                unread_count.load(std::memory_order_relaxed) == 0) {
                break;
            }
            // On attend un peu et on réessaie
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        int value = 0;
        {
            std::lock_guard<std::mutex> lock(read_mutex);
            value = buffer[read_pos];
            read_pos = (read_pos + 1) % BUFFER_SIZE;
        }

        unread_count.fetch_sub(1, std::memory_order_relaxed);
        semEmpty.release(); // une case libre de plus

        std::cout << "\tReader " << id << " <- " << value << std::endl;
        ++local_read;
    }

    std::cout << "\tReader " << id << " termine, valeurs lues = "
              << local_read << std::endl;
}

int main() {
    std::cout << "TP Sémaphore : " << NB_WRITERS << " writers, "
              << NB_READERS << " readers, buffer = " << BUFFER_SIZE
              << ", total valeurs = " << TOTAL_VALUES << std::endl;

    // Création des writers
    std::vector<std::thread> writers;
    writers.reserve(NB_WRITERS);
    for (int i = 0; i < NB_WRITERS; ++i) {
        writers.emplace_back(writer, i);   //chaque writer à un id unique
    }

    // Création des readers
    std::vector<std::thread> readers;
    readers.reserve(NB_READERS);
    for (int i = 0; i < NB_READERS; ++i) {
        readers.emplace_back(reader, i);  //chaque reader à un id unique
    } 

    // Attendre la fin de tous les writers
    for (auto &t : writers) {
        t.join();
    }
    writers_done.store(true, std::memory_order_release);

    // Attendre la fin de tous les readers
    for (auto &t : readers) {
        t.join();
    }

    std::cout << "Tous les threads ont terminé." << std::endl;
    return 0;
}