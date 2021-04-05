//
// Created by cpepi001 on 3/28/21.
//

#include "EOQTimer.H"
#include "RRScheduler.H"

extern RRScheduler *SYSTEM_SCHEDULER;

EOQTimer::EOQTimer(int EOQ) : SimpleTimer(EOQ) {
    Console::puts("Constructed EOQTimer.\n");
}

void EOQTimer::reset_quantum() {
    ticks = 0;
}

void EOQTimer::handle_interrupt(REGS *_r) {
    ticks++;

    if (ticks >= hz) {
        seconds++;
        ticks = 0;
        Console::puts("One quantum has passed. ");
        Console::puti(seconds);
        Console::puts(" in total\n");

        //Preempted
        SYSTEM_SCHEDULER->EOQ_handler();
    }
}