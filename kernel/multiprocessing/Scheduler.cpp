#include <multiprocessing/Scheduler.h>
#include <drivers/APIC.h>

extern "C"
{
    uint64_t *scheduler_nextThreadData;

    void scheduler_selectNext() {}

    void scheduler_sendEOI() 
    {
        Kernel::Drivers::APIC::Local()->SendEOI();
    }
}

namespace Kernel::Multiprocessing
{
    Scheduler localScheduler;
    Scheduler* Scheduler::The()
    {
        return &localScheduler;
    }

    
}