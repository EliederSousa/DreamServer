#include <thread>
#include <vector>
#include "include/threadpool.h"

ThreadPool::ThreadPool() {};

void ThreadPool::addThread( void (*funcThread)(int), int id ) {
    std::thread tempThread( funcThread, id );
    // move()
    // https://stackoverflow.com/a/30768282
    pool.push_back( move(tempThread) );
}

void ThreadPool::waitToFinish() {
    for( auto& t : pool ) {
        t.join();
    }
}

int ThreadPool::getSize() {
    return pool.size();
}