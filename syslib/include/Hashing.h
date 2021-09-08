#pragma once

#include <stdint-gcc.h>
#include <stddef.h>

namespace sl
{
    uint8_t CRC8(const void* start, const size_t count);
    uint64_t CRC64(const void* start, const size_t count);
}