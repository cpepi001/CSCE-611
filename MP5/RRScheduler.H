//
// Created by cpepi001 on 3/28/21.
//

#ifndef MP5_RRSCHEDULER_H
#define MP5_RRSCHEDULER_H

#include "EOQTimer.H"
#include "scheduler.H"

class RRScheduler : public Scheduler {
public:
    EOQTimer *eoq_timer;

    RRScheduler(int);

    void EOQ_handler();

};

#endif //MP5_RRSCHEDULER_H
