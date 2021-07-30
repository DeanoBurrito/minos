#include <multiprocessing/Thread.h>

namespace Kernel::Multiprocessing
{
    void KernelThread::Sleep()
    { }

    void KernelThread::Sleep(int64_t timeout)
    { }
    
    void KernelThread::Wake()
    { }

    int8_t KernelThread::GetPriority()
    { return 0; }

    void KernelThread::CancelTimerWakeup()
    { }
}