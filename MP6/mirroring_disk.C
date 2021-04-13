//
// Created by cpepi001 on 4/11/21.
//

#include "scheduler.H"
#include "mirroring_disk.H"

extern Scheduler *SYSTEM_SCHEDULER;

MirroringDisk::MirroringDisk(unsigned int _size) {
    primary = new SimpleDisk(MASTER, _size);
    secondary = new SimpleDisk(SLAVE, _size);
}

bool MirroringDisk::is_empty() {
    return blocked_queue.is_empty();
}

bool MirroringDisk::is_ready() {
    return ((Machine::inportb(0x1F7) & 0x08) != 0);;
}

Thread *MirroringDisk::get_thread() {
    return blocked_queue.dequeue();
}

void MirroringDisk::wait_until_ready() {
    if (!is_ready()) {
        Thread *thread = Thread::CurrentThread();

        Console::puts("Thread ");
        Console::puti(thread->ThreadId());
        Console::puts(" blocked\n");

        blocked_queue.enqueue(thread);
        SYSTEM_SCHEDULER->yield();
    }
}

void MirroringDisk::read(unsigned long _block_no, unsigned char *_buf) {
    issue_operation(MASTER, READ, _block_no);
    issue_operation(SLAVE, READ, _block_no);

    wait_until_ready();

    /* read data from port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i * 2] = (unsigned char) tmpw;
        _buf[i * 2 + 1] = (unsigned char) (tmpw >> 8);
    }
}

void MirroringDisk::write(unsigned long _block_no, unsigned char *_buf) {
    primary->write(_block_no, _buf);
    secondary->write(_block_no, _buf);
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
