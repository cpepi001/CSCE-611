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
    blocking_disk = NULL;

    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
//    if (Machine::interrupts_enabled())
//        Machine::disable_interrupts();

    if (blocking_disk != NULL) {
        if (!blocking_disk->is_blocked_queue_empty() && blocking_disk->is_ready()) {
            Thread *_thread = blocking_disk->get_blocked_thread();
            resume(_thread);
        }
    }

    if (ready_queue.is_empty())
        assert(false)

    Thread *_thread = ready_queue.dequeue();

//    Console::puts("Thread ");
//    Console::puti(_thread->ThreadId());
//    Console::puts(" acquired CPU\n");

    Thread::dispatch_to(_thread);

//    if (!Machine::interrupts_enabled())
//        Machine::enable_interrupts();
}

void Scheduler::resume(Thread *_thread) {
//    if (Machine::interrupts_enabled())
//        Machine::disable_interrupts();

    ready_queue.enqueue(_thread);

//    if (!Machine::interrupts_enabled())
//        Machine::enable_interrupts();
}

void Scheduler::add(Thread *_thread) {
    resume(_thread);
}

void Scheduler::terminate(Thread *_thread) {
//    if (Machine::interrupts_enabled())
//        Machine::disable_interrupts();

    ready_queue.delete_thread(_thread);

//    if (!Machine::interrupts_enabled())
//        Machine::enable_interrupts();
}

void Scheduler::add_disk(BlockingDisk *_disk) {
    if (blocking_disk == NULL) {
        blocking_disk = _disk;
    }
}
