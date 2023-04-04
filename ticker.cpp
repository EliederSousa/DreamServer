#include "include/ticker.h"
#include <chrono>

Ticker::Ticker() {
    tick = std::chrono::high_resolution_clock::now();
    counter = 0;
}

void Ticker::update() {
    tick = std::chrono::high_resolution_clock::now();
    counter++;
}

void Ticker::reset() {
    tick = std::chrono::high_resolution_clock::now();
    counter = 0;
}

double Ticker::compare() {
    return (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - tick)).count();
}

bool Ticker::compare( double timeToCompare ) {
    if( timeToCompare > (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - tick)).count() ) {
        return false;
    } else {
        tick = std::chrono::high_resolution_clock::now();
        counter++;
        return true;
    }
}

unsigned long long Ticker::getTicks() {
    return counter;
}