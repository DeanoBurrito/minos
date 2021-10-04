#pragma once

#include <stddef.h>
#include <stdint.h>

namespace sl
{
    class Bitmap
    {
    private:
        uint8_t* buffer;
        size_t bufferLength;
    
    public:
        Bitmap();
        Bitmap(size_t count);

        ~Bitmap();
        Bitmap(const Bitmap& other);
        Bitmap& operator=(const Bitmap& other);
        Bitmap(Bitmap&& from);
        Bitmap& operator=(Bitmap&& from);

        void Resize(size_t newCount);

        bool Get(size_t index);
        void Set(size_t index, bool status = true);
        void Clear(size_t index);
    };
}
