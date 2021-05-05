//
// Created by cpepi001 on 4/16/21.
//

#ifndef TAS_H
#define TAS_H


#include "thread.H"
#include "console.H"

struct Lock {
    unsigned int locked;
};

class TAS {
private:
    Lock lock;

    int TSL(unsigned int *addr);

public:

    TAS();

    void acquire();

    void release();

    unsigned int status();
};


#endif //TAS_H
