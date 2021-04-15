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

#include "tas.H"
#include "assert.H"
#include "utils.H"
#include "console.H"
#include "scheduler.H"
#include "blocking_disk.H"
/*--------------------------------------------------------------------------*/
/* SCHEDULER */
/*--------------------------------------------------------------------------*/

#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
TAS *tas;
#endif
#endif

extern Scheduler *SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
    tas = new TAS();
#endif
#endif
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

        Console::puts("\nThread ");
        Console::puti(thread->ThreadId());
        Console::puts(" get unblocked\n");

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
#ifdef _USES_SCHEDULER_
        SYSTEM_SCHEDULER->yield();
#endif
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
    issue_operation(READ, _block_no);

    wait_until_ready();

#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
    tas->acquire();
#endif
#endif
    /* read data from port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i * 2] = (unsigned char) tmpw;
        _buf[i * 2 + 1] = (unsigned char) (tmpw >> 8);
    }
#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
    tas->release();
#endif
#endif
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
    issue_operation(WRITE, _block_no);

    wait_until_ready();

#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
    tas->acquire();
#endif
#endif
    /* write data to port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2 * i] | (_buf[2 * i + 1] << 8);
        Machine::outportw(0x1F0, tmpw);
    }
#ifdef _BLOCKING_DISK_
#ifdef _THREAD_SAFE_
    tas->release();
#endif
#endif
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void BlockingDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no) {

    Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
    Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
    Machine::outportb(0x1F3, (unsigned char) _block_no);
    /* send low 8 bits of block number */
    Machine::outportb(0x1F4, (unsigned char) (_block_no >> 8));
    /* send next 8 bits of block number */
    Machine::outportb(0x1F5, (unsigned char) (_block_no >> 16));
    /* send next 8 bits of block number */
    Machine::outportb(0x1F6, ((unsigned char) (_block_no >> 24) & 0x0F) | 0xE0 | (MASTER << 4));
    /* send drive indicator, some bits,
       highest 4 bits of block no */

    Machine::outportb(0x1F7, (_op == READ) ? 0x20 : 0x30);

}
