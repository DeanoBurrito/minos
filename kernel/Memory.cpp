#include "Memory.h"

void memset(void* start, uint8_t value, uint64_t count)
{
    for (uint64_t i = 0; i < count; i++)
    {
        *(uint8_t*)((uint64_t)start + i) = value;
    } //TODO: lots of optimizations available here
}