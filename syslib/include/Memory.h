#pragma once

#include <stddef.h>
#include <stdint-gcc.h>

namespace sl
{
    template <typename T>
    T&& move(T&& t)
    {
        return static_cast<T&&>(t);
    }

    template <typename T>
    void swap(T& a, T& b)
    {
        T temp = move(a);
        a = move(b);
        b = move(temp);
    }
    
    //sets count bytes to value from start.
    void memset(void* const start, uint8_t value, size_t count);

    //copies count bytes from source to dest.
    void memcopy(const void* const source, void* const destination, size_t count);
    //copies count bytes from source to dest, with copy treating offsets as 'zero'. Operation is bounds checked.
    void memcopy(const void* const source, size_t sourceOffset, void* const dest, size_t destOffset, size_t count);

    //compares count bytes from a and b. Returns 0 if equal for count.
    int memcmp(const void* const a, const void* const b, size_t count);

    //returns the offset of the first instance of target from buff. UpperLimit is a max number of bytes to scan, 0 means no limit. 0 means target was not found.
    size_t memfirst(const void* const buff, uint8_t target, size_t upperLimit);
    //returns the offset of the first instance of target from (buff + offset). Upperlimit is max bytes to scan, 0 meaning no limit. Return of 0 means target was not found.
    size_t memfirst(const void* const buff, size_t offset, uint8_t target, size_t upperLimit);
}
