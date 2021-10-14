#pragma once

#include <stdint.h>
#include <DateTime.h>
#include <collections/List.h>

namespace Kernel::Drivers
{   
    struct ClockSourceStats
    {
        uint64_t errorFemtos; //as determined by our reference source
        uint64_t ticks;
        uint64_t tickFemtos; //real-time per tick
    };
    
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
        ClockSource mainSource;

    public:
        static SystemClock* The();

        uint64_t clockTicks;

        void Init();

        uint64_t GetUptime();
        sl::DateTime GetCurrentTime();
        ClockSource GetCurrentSource();

        sl::List<ClockSourceStats> GetStats();
    };
}
