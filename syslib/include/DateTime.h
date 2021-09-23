#pragma once

#include <stdint.h>

namespace sl
{
    class DateTime
    {
    private:
        uint64_t ticks;
        uint64_t sourceTicksPerSecond;
    public:
        //static DateTime FromSeconds(double seconds); //Requires FPU support TODO(fpu):
        /*
        static DateTime FromMillis(uint64_t millis);
        static DateTime FromMicros(uint64_t micros);
        static DateTime FromNanos(uint64_t nanos);
        static DateTime FromFemtos(uint64_t femtos);

        uint64_t ToMillis();
        uint64_t ToMicros();
        uint64_t ToNanos();
        uint64_t ToFemtos();
        */
    };
}