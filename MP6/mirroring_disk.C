//
// Created by cpepi001 on 4/11/21.
//

#include "tas.H"
#include "scheduler.H"
#include "mirroring_disk.H"

#ifdef _MIRRORING_DISK_
#ifdef _THREAD_SAFE_
TAS *tas;
#endif
#endif

extern Scheduler *SYSTEM_SCHEDULER;

MirroringDisk::MirroringDisk() {
#ifdef _MIRRORING_DISK_
#ifdef _THREAD_SAFE_
    tas = new TAS();
#endif
#endif
#ifdef _INTERRUPTS_
    InterruptHandler::register_handler(14, this);
#endif
}

bool MirroringDisk::is_empty() {
    return blocked_queue.is_empty();
}

bool MirroringDisk::is_ready() {
    return ((Machine::inportb(0x1F7) & 0x08) != 0);
}

Thread *MirroringDisk::get_thread() {
    return blocked_queue.dequeue();
}

void MirroringDisk::wait_until_ready() {
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

void MirroringDisk::handle_interrupt(REGS *_regs) {
#ifdef _INTERRUPTS_
    if (!is_empty() && is_ready()) {
        Thread *thread = blocked_queue.dequeue();

        Console::puts("\nThread ");
        Console::puti(thread->ThreadId());
        Console::puts(" get unblocked\n");

        SYSTEM_SCHEDULER->resume(thread);
    }
    Machine::outportb(0x20, 0x20);
#endif
}

void MirroringDisk::read(unsigned long _block_no, unsigned char *_buf) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    issue_operation(MASTER, READ, _block_no);
    issue_operation(SLAVE, READ, _block_no);

    wait_until_ready();

#ifdef _MIRRORING_DISK_
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
#ifdef _MIRRORING_DISK_
#ifdef _THREAD_SAFE_
    tas->release();
#endif
#endif
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void MirroringDisk::write(unsigned long _block_no, unsigned char *_buf) {
#ifdef _INTERRUPTS_
    if (Machine::interrupts_enabled())
        Machine::disable_interrupts();
#endif
    write(MASTER, _block_no, _buf);
    write(SLAVE, _block_no, _buf);
#ifdef _INTERRUPTS_
    if (!Machine::interrupts_enabled())
        Machine::enable_interrupts();
#endif
}

void MirroringDisk::write(DISK_ID _disk_id, unsigned long _block_no, unsigned char *_buf) {
    issue_operation(_disk_id, WRITE, _block_no);

    wait_until_ready();

#ifdef _MIRRORING_DISK_
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
#ifdef _MIRRORING_DISK_
#ifdef _THREAD_SAFE_
    tas->release();
#endif
#endif
}

void MirroringDisk::issue_operation(DISK_ID _disk_id, DISK_OPERATION _op, unsigned long _block_no) {
    Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
    Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
    Machine::outportb(0x1F3, (unsigned char) _block_no);
    /* send low 8 bits of block number */
    Machine::outportb(0x1F4, (unsigned char) (_block_no >> 8));
    /* send next 8 bits of block number */
    Machine::outportb(0x1F5, (unsigned char) (_block_no >> 16));
    /* send next 8 bits of block number */
    Machine::outportb(0x1F6, ((unsigned char) (_block_no >> 24) & 0x0F) | 0xE0 | (_disk_id << 4));
    /* send drive indicator, some bits,
       highest 4 bits of block no */

    Machine::outportb(0x1F7, (_op == READ) ? 0x20 : 0x30);
}
