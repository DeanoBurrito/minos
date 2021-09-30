#pragma once

#include <multiprocessing/Thread.h>
#include <templates/LinkedList.h>

extern "C"
{
    void scheduler_HandleInterrupt();
}

namespace Kernel::Multiprocessing
{
    class Scheduler
    {
    private:
        sl::LinkedList<Thread*> threads; //TODO: shouldnt be a pointer, emplace within list. This happens because we're using defaultValue.
        Thread* currentThread;

    public:
        static Scheduler* The();

        void Init();
        void SelectNext();
        void Yield();
    };
}