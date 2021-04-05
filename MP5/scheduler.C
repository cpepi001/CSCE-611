/*
 File: scheduler.C
 
 Author: Chrysanthos Pepi
 Date  : 24 MAR 2021
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();

    if (ready_queue.isEmpty())
        assert(false)

    Thread *_thread = ready_queue.dequeue();

//    Console::puts("Thread ");
//    Console::puti(_thread->ThreadId());
//    Console::puts(" acquired CPU\n");

    Thread::dispatch_to(_thread);

    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
}

void Scheduler::resume(Thread *_thread) {
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();

    ready_queue.enqueue(_thread);

    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
}

void Scheduler::add(Thread *_thread) {
    resume(_thread);
}

void Scheduler::terminate(Thread *_thread) {
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();

    ready_queue.deleteThread(_thread);

    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
}
