#include <DateTime.h>

namespace Syslib
{
    uint64_t DateTime::ticksPerSecond = 1; //will be set by the selected system clocksource

    void DateTime::FromNanos(uint64_t nanos)
    {
        ticks = ((ticksPerSecond * 1'000'000'000) * nanos / 1'000'000'000);
    }

    void DateTime::FromMicros(uint64_t micros)
    {
        ticks = ((ticksPerSecond * 1'000'000) * micros / 1'000'000);
    }

    void DateTime::FromMillis(uint64_t millis)
    {
        ticks = ((ticksPerSecond * 1000) * millis / 1000);
    }

    uint64_t DateTime::ToNanos()
    {
        uint64_t divisor = ticksPerSecond * 1'000'000'000;
        return ticks / divisor;
    }

    uint64_t DateTime::ToMicros()
    {
        uint64_t divisor = ticksPerSecond * 1'000'000;
        return ticks / divisor;
    }

    uint64_t DateTime::ToMillis()
    {
        uint64_t divisor = ticksPerSecond * 1'000;
        return ticks / divisor;
    }

}