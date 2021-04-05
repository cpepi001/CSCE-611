//
// Created by cpepi001 on 3/28/21.
//

#include "RRScheduler.H"

RRScheduler::RRScheduler(int EOQ) {
    eoq_timer = new EOQTimer(EOQ); /* timer ticks every 50ms. */
    InterruptHandler::register_handler(0, eoq_timer);
    Console::puts("Constructed RRScheduler.\n");
}

void RRScheduler::EOQ_handler() {
//    Console::puts("Thread ");
//    Console::puti(Thread::CurrentThread()->ThreadId());
//    Console::puts(" preempted\n");
    resume(Thread::CurrentThread());
    Machine::outportb(0x20, 0x20);
    yield();
}
