//
// Created by cpepi001 on 3/24/21.
//

#ifndef QUEUE_H
#define QUEUE_H

#include "assert.H"
#include "console.H"
#include "thread.H"
#include "utils.H"

class Queue {
private:
    int size;
    Queue *queue;
    Thread *thread;

public:
    Queue();

    Queue(Thread *);

    Thread *dequeue();

    void enqueue(Thread *);

    void delete_thread(Thread *);

    void print();

    bool is_empty();
};

#endif
