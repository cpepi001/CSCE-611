//
// Created by cpepi001 on 4/11/21.
//

#ifndef MP6_MIRRORING_DISK_H
#define MP6_MIRRORING_DISK_H

#include "queue.H"
#include "simple_disk.H"

class MirroringDisk {
private:
    Queue blocked_queue;
    SimpleDisk *primary;
    SimpleDisk *secondary;

public:
    MirroringDisk(unsigned int _size);

    bool is_ready();

    bool is_empty();

    Thread *get_thread();

    void wait_until_ready();

    void read(unsigned long _block_no, unsigned char *_buf);

    void write(unsigned long _block_no, unsigned char *_buf);

    void issue_operation(DISK_ID _disk_id, DISK_OPERATION _op, unsigned long _block_no);
};


#endif //MP6_MIRRORING_DISK_H
