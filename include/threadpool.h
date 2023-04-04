#ifndef _SERVER_THREADPOOL_
#define _SERVER_THREADPOOL_

#include <thread>
#include <vector>

class ThreadPool {
    private:
        std::vector<std::thread> pool;

    public:
        ThreadPool();
        void addThread( void (*funcThread)(int), int id );
        void waitToFinish();
        int getSize();
};

#endif