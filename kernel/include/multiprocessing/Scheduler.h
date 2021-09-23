#pragma once

#include <multiprocessing/Thread.h>

extern "C"
{
    void scheduler_HandleInterrupt();
}

namespace Kernel::Multiprocessing
{
    class Scheduler
    {
    private:

    public:
        static Scheduler* The();

        void Init();
        void Yield();
    };
}