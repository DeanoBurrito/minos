#pragma once

#include <stddef.h>
#include <stdint-gcc.h>

namespace Syslib
{
    class Buffer;
    typedef const Buffer ReadOnlyBuffer; //can always specialize this later, for now setting as const is enough

    class Buffer
    {
    private:
        uint8_t* start;
        size_t length;

    public:
        static ReadOnlyBuffer* CopyToReadOnly(const Buffer* const from, size_t offset, size_t length);
        static ReadOnlyBuffer* CopyToReadOnly(const unsigned char* const from, size_t offset, size_t length);
        static Buffer* CopyTo(const Buffer* const from, size_t offset, size_t length);
        static Buffer* CopyTo(const unsigned char* const from, size_t offset, size_t length);

        //if true it will act the exclusive owner of memory (ie freeing it when needed)
        bool owning = true;

        Buffer();
        Buffer(uint8_t* start, size_t length);
        Buffer(uint8_t* start, size_t length, uint8_t fillWith);
        Buffer(size_t reserveSize);

        ~Buffer();
        Buffer(const Buffer& copy);
        Buffer& operator=(const Buffer& copy);
        Buffer(Buffer&& from);
        Buffer& operator=(Buffer&& from);

        void Resize(size_t newSize);

        void Set(size_t offset, uint8_t data);
        uint8_t& At(size_t offset) const;
        size_t Size() const;
        uint8_t* const Data();
        const uint8_t* const Data() const;

        uint8_t& operator[](size_t index) const;
    };
    
}