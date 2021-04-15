//
// Created by cpepi001 on 4/16/21.
//

#include "tas.H"
#include "scheduler.H"

extern Scheduler *SYSTEM_SCHEDULER;

TAS::TAS() {
    lock.locked = false;
}

void TAS::acquire() {
    while (TSL((&(lock.locked))) != 0) {
        Thread *thread = Thread::CurrentThread();

        Console::puts("Thread ");
        Console::puti(thread->ThreadId());
        Console::puts(" is waiting the lock..\n");

#ifdef _USES_SCHEDULER_
        SYSTEM_SCHEDULER->resume(thread);
        SYSTEM_SCHEDULER->yield();
#endif
    }
    Console::puts("Locked acquired\n");
}

void TAS::release() {
    lock.locked = 0;
    Console::puts("Locked released\n");
}

int TAS::TSL(unsigned int *addr) {
    register int content = 1;
    asm volatile ("xchgl %0,%1" : "=r" (content),
    "=m" (*addr) : "0" (content), "m" (*addr));
    return (content);
}

unsigned int TAS::status() {
    return lock.locked;
}
