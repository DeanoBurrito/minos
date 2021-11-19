#pragma once

#include <stdint.h>
#include <stddef.h>

namespace sl::Hashes
{
    uint8_t CRC8(const void* start, const size_t count);
    uint64_t CRC64(const void* start, const size_t count);

    constexpr uint32_t Simple32(uint32_t key)
    {
        key += ~(key << 15);
        key ^= (key >> 10);
        key += (key << 3);
        key ^= (key >> 6);
        key += ~(key << 11);
        key ^= (key >> 16);
        return key;
    }

    constexpr uint32_t Simple64(uint64_t key)
    {
        uint32_t low = key & 0xFFFF'FFFF;
        uint32_t high = key >> 32;
        return Simple32((Simple32(low) * 209) ^ (Simple32(high) * 413));
    }

    constexpr uint32_t DoubleSimple(uint32_t key)
    {
        key = ~key + (key >> 23);
        key ^= (key << 12);
        key ^= (key >> 7);
        key ^= (key << 2);
        key ^= (key >> 20);
        return key;
    }

    constexpr uint32_t PairSimple(uint32_t a, uint32_t b)
    {
        return Simple32((Simple32(a) * 209) ^ (Simple32(b) * 413));
    }
}