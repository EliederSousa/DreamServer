#ifndef _SERVER_TICKER_
#define _SERVER_TICKER_

#include <chrono>

class Ticker {
    private:
        unsigned long long counter;
        std::chrono::high_resolution_clock::time_point tick;
        
    public:
        Ticker();
        void update();
        void reset();
        double compare();
        bool compare( double timeToCompare );
        unsigned long long getTicks();
};

#endif