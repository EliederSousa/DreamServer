#ifndef _SERVER_UTILS_
#define _SERVER_UTILS_

class IDGenerator {    
    public: 
    static int generate() {
        static int _uniqueid = 1;
        return _uniqueid++;
    }  
};

#endif