#pragma once

#include <CppStd.h>
#include <stdint.h>
#include <Hashing.h>

namespace sl
{
    template<typename T>
    struct GenericTraits
    {
        using PeekType = T;
        using ConstPeekType = T;

        static constexpr bool IsTrivial() { return false; }
        static constexpr bool Equals(const T& a, const T& b) { return a == b; }
    };

    template<typename T>
    struct Traits : public GenericTraits<T>
    {};

    template<>
    struct Traits<int> : public GenericTraits<int>
    {
        static constexpr bool IsTrivial() { return true; }
        static constexpr uint32_t Hash(int value) { return Hashes::Simple32(value); }
    };
}
