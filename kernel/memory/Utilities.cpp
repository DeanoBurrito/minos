#include <memory/Utilities.h>

void memset(void* const start, uint8_t value, uint64_t count)
{
    for (uint64_t i = 0; i < count; i++)
    {
        *(uint8_t*)((uint64_t)start + i) = value;
    } //TODO: lots of optimizations available here
}

void memcopy(const void* const source, void* const dest, uint64_t count)
{
    const uint8_t* const si = reinterpret_cast<const uint8_t* const>(source);
    uint8_t* di = reinterpret_cast<uint8_t*>(dest);
    for (uint64_t i = 0; i < count; i++)
        di[i] = si[i];
}

int memcmp(const void* const a, const void* const b, uint64_t count)
{
    const uint8_t* const ai = reinterpret_cast<const uint8_t* const>(a);
    const uint8_t* const bi = reinterpret_cast<const uint8_t* const>(b);

    for (int i = 0; i < count; i++)
    {
        if (ai[i] > bi[i])
            return 1;
        else if (bi[i] > ai[i])
            return -1;
    }
    return 0;
}
