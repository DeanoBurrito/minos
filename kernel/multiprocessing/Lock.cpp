#include <multiprocessing/Lock.h>

namespace Kernel::Multiprocessing
{
    ScopedSpinlock::ScopedSpinlock(void* l)
    {
        lock = l;
        SpinlockTake(lock);
    }

    ScopedSpinlock::~ScopedSpinlock()
    {
        SpinlockRelease(lock);
    }
}
