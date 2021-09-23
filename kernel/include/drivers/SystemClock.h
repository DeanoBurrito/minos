#pragma once

#include <stdint.h>
#include <DateTime.h>

namespace Kernel::Drivers
{   
    enum class ClockSource
    {
        X86_PIT,
        X86_HPET,
        X86_TSC,
    };
    
    /*
        Abstraction layer over specific timers. Allows for easily getting time and date from the most appropriate device.
    */
    class SystemClock
    {
    private:
    public:
        static SystemClock* The();

        uint64_t clockTicks;

        void Init(ClockSource source);

        uint64_t GetUptime();
        sl::DateTime GetCurrentTime();
    };
}
