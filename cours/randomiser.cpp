#include <iostream>
#include <random>

int main(void) {
    std::mt19937 gen(std::random_device{}());              // générateur
    std::uniform_int_distribution<int> dis(1, 10);       // distribution

    std::cout << "ceci est un test de randomnisation" << dis(gen);
    return 0;
}