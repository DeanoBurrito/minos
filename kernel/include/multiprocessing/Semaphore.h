#pragma once

#include <stdint.h>
#include <templates/LinkedList.h>
#include <multiprocessing/Thread.h>

#define SEMAPHORE_TIMEOUT_FOREVER -1
#define SEMAPHORE_TIMEOUT_NONE 0

namespace Kernel::Multiprocessing
{
    class Semaphore
    {
    private:
        sl::LinkedList<KernelThread*> waitingThreads; //TODO: better data structure than linked list
        uint64_t value;

    public:
        Semaphore(const uint64_t initialCount = 0);
        
        void Up(const uint64_t amount = 1);
        bool Down(const uint64_t amount = 1, int64_t timeout = SEMAPHORE_TIMEOUT_FOREVER);
        uint64_t Count();
    };
}