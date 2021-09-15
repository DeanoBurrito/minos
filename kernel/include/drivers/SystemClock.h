#pragma once

#include <stdint.h>
#include <stddef.h>
#include <templates/List.h>

namespace Kernel::Drivers
{
    enum class ClockSourceType
    {
        X86_TSC,
        X86_LAPIC,
        X86_LAPIC_TSC,
        X86_PIT,
        X86_HPET,

        ARM_GENERIC_TIMER,
        ARM_LOCAL_TIMER,
        ARM_SYSTEM_TIMER,
    };
    
    class SystemClock
    {
    private:
        sl::List<ClockSourceType> availableSources;
        ClockSourceType primarySource;

    public:
        void Init();
        bool SourceAvailable(ClockSourceType type);

        //Sets the source used for system clock ticks (unrelated to scheduler - nor used for setting time, only advancing it).
        void SelectPrimarySource(ClockSourceType type);

        //Gets the number of counting units for a given source (assume single-shot and periodic operation available)
        size_t GetSourceTimers(ClockSourceType type);

        //Sets up a timer for single-shot mode. Timer will not begin immmediately if possible.
        bool SetupSingleShotIRQ(ClockSourceType type, size_t timerIndex, uint8_t vector, uint64_t millis);

        //Sets up a timer to trigger an IRQ on a specific millisecond interval. Timer is masked by default. Returns if operation was successful
        bool SetupTimerIRQ(ClockSourceType type, size_t timerIndex, uint8_t vector, uint64_t millis);
    };
}