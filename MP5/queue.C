//
// Created by cpepi001 on 3/24/21.
//

#include "queue.H"

Queue::Queue() {
    size = 0;
    queue = NULL;
    thread = NULL;
    Console::puts("Queue initialized. Yeah...\n");
}

Queue::Queue(Thread *_thread) {
    queue = NULL;
    thread = _thread;
}

Thread *Queue::dequeue() {
    if (thread == NULL)
        return NULL;

    Thread *_thread = thread;
    if (queue != NULL) {
        thread = queue->thread;
        Queue *temp_queue = queue;
        queue = queue->queue;

        delete[] temp_queue;
    } else {
        thread = NULL;
    }
    size--;

//    Console::puts("Thread with ID: ");
//    Console::puti(thread->ThreadId());
//    Console::puts(" was dequeued\n");
    return _thread;
}

void Queue::enqueue(Thread *_thread) {
    if (thread == NULL) {
        thread = _thread;
    } else {
        if (queue == NULL) {
            queue = new Queue(_thread);
        } else {
            queue->enqueue(_thread);
        }
    }
    size++;

//    Console::puts("Thread with ID: ");
//    Console::puti(_thread->ThreadId());
//    Console::puts(" was enqueued\n");
}

void Queue::deleteThread(Thread *_thread) {
    for (int i = 0; i < size; ++i) {
        Thread *temp_thread = dequeue();
        if (temp_thread->ThreadId() != _thread->ThreadId()) {
            enqueue(temp_thread);
        } else {
            size--;
            break;
        }
    }
}

void Queue::print() {
    Queue *temp_queue = queue;
    Thread *temp_thread = thread;

    for (int i = 0; i < size; ++i) {
        Console::puts("Thread ");
        Console::puti(temp_thread->ThreadId());

        if (temp_queue->queue->thread != NULL) {
            Console::puts(" is pointing to Thread ");
            Console::puti(temp_queue->thread->ThreadId());
        } else {
            Console::puts(" is pointing to NULL");
        }
        Console::puts("\n");

        temp_thread = temp_queue->thread;
        temp_queue = temp_queue->queue;
    }
    Console::puts("------------------------------\n");
}

bool Queue::isEmpty() {
    return size == 0;
}

