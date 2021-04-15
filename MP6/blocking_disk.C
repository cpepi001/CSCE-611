/*
     File        : blocking_disk.c

     Author      : Chrysanthos Pepi
     Modified    : 08 APR 21

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "scheduler.H"
#include "blocking_disk.H"
/*--------------------------------------------------------------------------*/
/* SCHEDULER */
/*--------------------------------------------------------------------------*/

extern Scheduler *SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
#ifdef _INTERRUPTS_
    InterruptHandler::register_handler(14, this);
#endif
}

Thread *BlockingDisk::get_thread() {
    return blocked_queue.dequeue();
}

bool BlockingDisk::is_empty() {
    return blocked_queue.is_empty();
}

void BlockingDisk::handle_interrupt(REGS *_regs) {
#ifdef _INTERRUPTS_
    if (!is_empty() && is_ready()) {
        Thread *thread = blocked_queue.dequeue();

//        Console::puts("\nThread ");
//        Console::puti(thread->ThreadId());
//        Console::puts(" get unblocked\n");

        SYSTEM_SCHEDULER->resume(thread);
    }
#endif
}

void BlockingDisk::wait_until_ready() {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    if (!is_ready()) {
        Thread *thread = Thread::CurrentThread();

        Console::puts("Thread ");
        Console::puti(thread->ThreadId());
        Console::puts(" blocked\n");

        blocked_queue.enqueue(thread);
        SYSTEM_SCHEDULER->yield();
    }
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

bool BlockingDisk::is_ready() {
    return SimpleDisk::is_ready();
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char *_buf) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    SimpleDisk::read(_block_no, _buf);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void BlockingDisk::write(unsigned long _block_no, unsigned char *_buf) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    SimpleDisk::write(_block_no, _buf);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}
