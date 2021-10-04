#pragma once

#include <stdint.h>

__attribute__((always_inline, unused)) inline static uint32_t FloatToBits(float arg)
{
    union { float f; uint32_t i; } un;
    un.f = arg;
    return un.i;
}

__attribute__((always_inline, unused)) inline static uint64_t DoubleToBits(double arg)
{
    union { double d; uint64_t i; } un;
    un.d = arg;
    return un.i;
}

__attribute__((always_inline, unused)) inline static float BitsToFloat(uint32_t arg)
{
    union { uint32_t i; float f; } un;
    un.i = arg;
    return un.f;
}

__attribute__((always_inline, unused)) inline static float BitsToDouble(uint64_t arg)
{
    union { uint64_t i; double d; } un;
    un.i = arg;
    return un.d;
}

#define IsNan(num) \
(   \
    sizeof(num) == sizeof(float) ? (FloatToBits(num) & 0x7FFF'FFFF) > 0x7F80'0000 : \
    sizeof(num) == sizeof(double) ? (DoubleToBits(num) & -1ULL >> 1) > 0x7FFULL << 52 : \
    false \
)

#define IsInfinite(num) \
(   \
    sizeof(num) == sizeof(float) ? (FloatToBits(num) & 0x7FFF'FFFF) == 0x7F80'0000 : \
    sizeof(num) == sizeof(double) ? (DoubleToBits(num) & -1ULL >> 1) == 0x7FFULL << 52 : \
    false \
)

#define Min(a, b) ( (a) < (b) ? (a) : (b) )
#define Max(a, b) ( (a) > (b) ? (a) : (b) )

#define FemtoToMillis(femto) ((femto) / 1'000'000'000'000)
#define MilliToFemtos(milli) ((milli) * 1'000'000'000'000)
