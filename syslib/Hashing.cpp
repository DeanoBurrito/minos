#include <Hashing.h>

namespace sl
{
    uint8_t CRC8(const void* start, const size_t count)
    {
        uint8_t counter = 0;
        const uint8_t* startPtr = reinterpret_cast<const uint8_t*>(start);
        for (size_t i = 0; i < count; i++)
            counter += (i + 1) * startPtr[count];
        
        return counter;
    }

    uint64_t CRC64(const void* start, const size_t count)
    {
        uint64_t counter = 0;
        const uint8_t* startPtr = reinterpret_cast<const uint8_t*>(start);
        for (size_t i = 0; i < count; i++)
            counter += (i + 1) * startPtr[count];
        
        return counter;
    }
}