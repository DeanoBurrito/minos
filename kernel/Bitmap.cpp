#include <Bitmap.h>

namespace Kernel
{
    bool Bitmap::operator[](uint64_t index)
    {
        if (index > size * 8)
            return false;

        uint64_t byteIndex = index / 8;
        uint8_t bitIndex = index % 8;
        uint8_t mask = 0b10000000 >> bitIndex;

        return (buffer[byteIndex] & mask) > 0;
    }

    bool Bitmap::Get(uint64_t index)
    {
        return operator[](index);
    }

    bool Bitmap::Set(uint64_t index, bool value)
    {
        if (index > size * 8)
            return false;

        uint64_t byteIndex = index / 8;
        uint8_t bitIndex = index % 8;
        uint8_t mask = 0b10000000 >> bitIndex;

        //clear bit, and set high if required, otherwise leave it
        buffer[byteIndex] &= ~mask;
        if (value)
            buffer[byteIndex] |= mask;
        return true;
    }
}
