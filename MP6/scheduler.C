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
#ifdef _BLOCKING_DISK_
    disk = NULL;
#endif

#ifdef _MIRRORING_DISK_
    disk = NULL;
#endif

    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif

#ifdef _BLOCKING_DISK_
    #ifndef _INTERRUPTS_
    if (!disk->is_empty() && disk->is_ready()) {
        Thread *_thread = disk->get_thread();
        resume(_thread);
    }
#endif
#endif

#ifdef _MIRRORING_DISK_
    #ifndef _INTERRUPTS_
    if (!disk->is_empty() && disk->is_ready()) {
        Thread *_thread = disk->get_thread();
        resume(_thread);
    }
#endif
#endif

    if (ready_queue.is_empty())
        assert(false)

    Thread *_thread = ready_queue.dequeue();

//    Console::puts("Thread ");
//    Console::puti(_thread->ThreadId());
//    Console::puts(" acquired CPU\n");

    Thread::dispatch_to(_thread);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void Scheduler::resume(Thread *_thread) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    ready_queue.enqueue(_thread);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void Scheduler::add(Thread *_thread) {
    resume(_thread);
}

void Scheduler::terminate(Thread *_thread) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    ready_queue.delete_thread(_thread);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

#ifdef _BLOCKING_DISK_
void Scheduler::add_disk(BlockingDisk *_disk) {
    if (disk == NULL) {
        disk = _disk;
    }
}
#endif

#ifdef _MIRRORING_DISK_
void Scheduler::add_disk(MirroringDisk *_disk) {
    if (disk == NULL) {
        disk = _disk;
    }
}
#endif