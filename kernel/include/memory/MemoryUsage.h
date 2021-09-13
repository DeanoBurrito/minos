#pragma once

#include <stddef.h>

namespace Kernel::Memory
{
    struct MemoryUsage
    {
        MemoryUsage(const size_t nTotal, const size_t nFree, const size_t nReserved, const size_t nUsed)
            : total{nTotal}, free{nFree}, reserved{nReserved}, used{nUsed}
        {
        }

        const size_t total;
        const size_t free;
        const size_t reserved;
        const size_t used;
    };
}