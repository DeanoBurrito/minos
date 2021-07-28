#include <memory/Utilities.h>

void memset(void* start, uint8_t value, uint64_t count)
{
    for (uint64_t i = 0; i < count; i++)
    {
        *(uint8_t*)((uint64_t)start + i) = value;
    } //TODO: lots of optimizations available here
}

void memcopy(void* source, void* dest, uint64_t count)
{
    uint8_t* si = reinterpret_cast<uint8_t*>(source);
    uint8_t* di = reinterpret_cast<uint8_t*>(dest);
    for (uint64_t i = 0; i < count; i++)
        di[i] = si[i];
}
