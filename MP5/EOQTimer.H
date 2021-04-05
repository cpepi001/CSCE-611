//
// Created by cpepi001 on 3/28/21.
//

#ifndef MP5_EOQTIMER_H
#define MP5_EOQTIMER_H

#include "console.H"
#include "simple_timer.H"

class EOQTimer : public SimpleTimer {

public:
    EOQTimer(int);

    void reset_quantum();

    void handle_interrupt(REGS *);
};


#endif //MP5_EOQTIMER_H
