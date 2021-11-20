#pragma once

#include <Platform.h>

namespace Kernel::Multiprocessing
{
    //busy waits until lock is cleared, then takes the lock. Should be used with SpinlockRelease()
    FORCE_INLINE static void SpinlockTake(void* lock)
    {
        while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
    }

    //releases a current held spinlock
    FORCE_INLINE static void SpinlockRelease(void* lock)
    {
        __atomic_clear(lock, __ATOMIC_RELEASE);
    }

    //ctor halts execution until lock can be taken, dtor releases it
    class ScopedSpinlock
    {
    private:
        void* lock;

    public:
        ScopedSpinlock(void* lock);

        ~ScopedSpinlock();
        ScopedSpinlock(const ScopedSpinlock& other) = delete;
        ScopedSpinlock& operator=(const ScopedSpinlock& other) = delete;
        ScopedSpinlock(ScopedSpinlock&& from) = delete;
        ScopedSpinlock& operator=(ScopedSpinlock&& from) = delete;
    };
}