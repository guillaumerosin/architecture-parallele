#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <atomic>

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// TP Variable conditionnelle — Slide 65
// - 3 threads qui élèvent x à la puissance y et affichent le résultat
// - le main demande en boucle x, y et si l'utilisateur veut continuer
// - notify_one()  → réveille UN thread pour effectuer le calcul
// - notify_all()  → réveille TOUS les threads pour qu'ils se terminent
// ─────────────────────────────────────────────────────────────────────────────

// Mutex obligatoire pour la variable conditionnelle (slide 60)
mutex m;

// Variable conditionnelle (slide 60)
condition_variable cv;

// Flag de réveil — évite spurious wakeup et notification manquée (slide 61)
bool go = false;

// Flag de fin — les threads doivent se terminer
bool fin = false;

// Valeurs partagées x et y demandées par le main
double x_val = 0;
double y_val = 0;

// Mutex pour protéger l'affichage
mutex screen_mutex;

// ─────────────────────────────────────────────────────────────────────────────
// Fonction des threads — attend un signal, calcule x^y, affiche le résultat
// ─────────────────────────────────────────────────────────────────────────────
void thread_fct(int id)
{
    while (true)
    {
        // ── Attente du signal (slide 60) ──────────────────────────────────
        unique_lock<mutex> lock(m);

        // cv.wait avec lambda — évite spurious wakeup (slide 61)
        // Le thread se rendort si go = false
        cv.wait(lock, [] { return go || fin; });

        // Si fin = true → le thread se termine proprement
        if (fin)
        {
            screen_mutex.lock();
            cout << "Thread[" << id << "] : fin du thread" << endl;
            screen_mutex.unlock();
            return;
        }

        // Récupérer les valeurs avant de libérer le mutex
        double x = x_val;
        double y = y_val;

        // Remettre go à false pour que les autres threads
        // ne calculent pas avec les mêmes valeurs (notify_one)
        go = false;

        // Libération manuelle du mutex AVANT le calcul
        // → les autres threads peuvent continuer (slide 64)
        lock.unlock();

        // ── Calcul x^y ───────────────────────────────────────────────────
        double result = pow(x, y);

        // ── Affichage protégé ─────────────────────────────────────────────
        screen_mutex.lock();
        cout << "Thread[" << id << "] : "
             << x << "^" << y << " = " << result << endl;
        screen_mutex.unlock();
    }
}

int main()
{
    // Création des 3 threads (slide 26)
    vector<thread> threads;
    threads.reserve(3);
    for (int i = 1; i <= 3; i++)
        threads.emplace_back(thread_fct, i);

    // Boucle principale du main
    while (true)
    {
        // Demander x et y à l'utilisateur
        cout << "x = ";
        cin >> x_val;
        cout << "y = ";
        cin >> y_val;

        // Réveiller UN thread pour effectuer le calcul (slide 60 + 62)
        {
            lock_guard<mutex> lock(m);
            go = true;
        }
        cv.notify_one();   // réveille un seul thread en attente

        // Demander si l'utilisateur veut continuer
        int continuer = 1;
        cout << "Autre calcul ? (0=non / 1=oui) ";
        cin >> continuer;

        if (continuer == 0)
            break;
    }

    // ── Fin du programme — réveiller tous les threads (slide 62) ─────────
    {
        lock_guard<mutex> lock(m);
        fin = true;
    }
    cv.notify_all();   // réveille tous les threads en attente

    // Attendre que tous les threads se terminent (slide 26)
    for (auto& t : threads)
        t.join();

    return 0;
}