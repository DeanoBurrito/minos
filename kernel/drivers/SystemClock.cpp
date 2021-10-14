#include <drivers/SystemClock.h>

namespace Kernel::Drivers
{
    SystemClock globalSystemClock;
    SystemClock* SystemClock::The()
    {
        return &globalSystemClock;
    }
    
    void SystemClock::Init()
    {
        /*
            Iterate through preferred clock sources:
            -   We need to know if HPET is available, we calibrate against that.
                Otherwise it'll be the PIT.
            -   if HPET is available, bring it online.
            -   Now we can calibrate the apic using the selected ref source.
            -   Set a callback for inside of apic_timer callback if we want ot use that
                (itll be shared with scheduling)
            -   Otherwise fallback to HPET or PIT depending on if HPET is avaiable or not.
        */

        //going to switch handlers to operating on function tables:
        //we pass in an array of pointers, timer callback first checks which ones are valid, and then calls them
        //TODO: we'll need non-exclusive irqs for this to function properly
    }

    uint64_t SystemClock::GetUptime()
    {
        return clockTicks;
    }

    sl::DateTime SystemClock::GetCurrentTime()
    {
        return {};
    }

    ClockSource SystemClock::GetCurrentSource()
    {
        return mainSource;
    }

    sl::List<ClockSourceStats> GetStats()
    {
        return {};
    }
}
