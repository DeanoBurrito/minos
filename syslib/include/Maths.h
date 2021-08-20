#pragma once

#include <stdint.h>

__attribute__((always_inline)) static uint32_t Float32Bits(float arg)
{
    union { float f; uint32_t i; } un;
    un.f = arg;
    return un.i;
}

__attribute__((always_inline)) static uint64_t Float64Bits(double arg)
{
    union { double d; uint64_t i; } un;
    un.d = arg;
    return un.i;
}

#define IsNan(num) \
(   \
    sizeof(num) == sizeof(float) ? (Float32Bits(num) & 0x7FFF'FFFF) > 0x7F80'0000 : \
    sizeof(num) == sizeof(double) ? (Float64Bits(num) & -1ULL >> 1) > 0x7FFULL << 52 : \
    false \
)

#define IsInfinite(num) \
(   \
    sizeof(num) == sizeof(float) ? (Float32Bits(num) & 0x7FFF'FFFF) == 0x7F80'0000 : \
    sizeof(num) == sizeof(double) ? (Float64Bits(num) & -1ULL >> 1) == 0x7FFULL << 52 : \
    false \
)

#define Min(a, b) ( (a) < (b) ? (a) : (b) )
#define Max(a, b) ( (a) > (b) ? (a) : (b) )

#define FemtoToMillis(femto) ((femto) / 1'000'000'000'000)
#define MilliToFemtos(milli) ((milli) * 1'000'000'000'000)
