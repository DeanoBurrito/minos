#pragma once

#include <multiprocessing/Thread.h>

namespace Kernel::Multiprocessing
{
    class Scheduler
    {
    private:
    public:
        static Scheduler* The();
        
        KernelThread* GetCurrentThread();
    };
}