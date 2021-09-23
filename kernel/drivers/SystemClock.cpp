#include <drivers/SystemClock.h>

namespace Kernel::Drivers
{
    SystemClock globalSystemClock;
    SystemClock* SystemClock::The()
    {
        return &globalSystemClock;
    }
    
    void SystemClock::Init(ClockSource source)
    {
        //Validate clocksource is available and initialized
    }

    uint64_t SystemClock::GetUptime()
    {
        return clockTicks;
    }

    sl::DateTime SystemClock::GetCurrentTime()
    {
        return {};
    }
}
