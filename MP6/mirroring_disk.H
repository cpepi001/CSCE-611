//
// Created by cpepi001 on 4/11/21.
//

#ifndef _MIRRORING_DISK_H
#define _MIRRORING_DISK_H

#include "queue.H"
#include "interrupts.H"
#include "simple_disk.H"

class MirroringDisk : public InterruptHandler {
private:
    Queue blocked_queue;

    void write(DISK_ID _disk_id, unsigned long _block_no, unsigned char *_buf);

public:
    MirroringDisk();

    bool is_ready();

    bool is_empty();

    Thread *get_thread();

    void wait_until_ready();

    void handle_interrupt(REGS *_regs);

    void read(unsigned long _block_no, unsigned char *_buf);

    void write(unsigned long _block_no, unsigned char *_buf);

    void issue_operation(DISK_ID _disk_id, DISK_OPERATION _op, unsigned long _block_no);
};


#endif //_MIRRORING_DISK_H
