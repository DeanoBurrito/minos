#pragma once

#include <stdint-gcc.h>

namespace Kernel::Multiprocessing
{
    class KernelThread
    {
    private:
    public:
        void Sleep();
        void Sleep(int64_t timeout);
        void Wake();
        int8_t GetPriority();

        void CancelTimerWakeup();
    };
}