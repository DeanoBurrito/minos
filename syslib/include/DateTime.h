#pragma once

#include <stdint.h>

namespace sl
{
    class DateTime
    {
    private:
        static uint64_t ticksPerSecond;

    public:
        //number of system-dependant intervals since epoc
        uint64_t ticks;

        void FromNanos(uint64_t nanos);
        void FromMicros(uint64_t micros);
        void FromMillis(uint64_t millis);

        uint64_t ToNanos();
        uint64_t ToMicros();
        uint64_t ToMillis();
        //double seconds(); //TOOD: enable FPU and setup task switching for it
    };
}