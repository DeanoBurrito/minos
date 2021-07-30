#include <multiprocessing/Scheduler.h>

namespace Kernel::Multiprocessing
{
    Scheduler schedulerInstance;
    Scheduler* Scheduler::The()
    {
        return &schedulerInstance;
    }

    KernelThread* Scheduler::GetCurrentThread()
    { return nullptr; }
}