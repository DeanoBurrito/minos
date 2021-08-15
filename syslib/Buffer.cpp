#include <Buffer.h>
#include <Memory.h>

namespace sl
{
    ReadOnlyBuffer* Buffer::CopyToReadOnly(const Buffer* const from, size_t offset, size_t length)
    {
        return CopyTo(from, offset, length);
    }

    ReadOnlyBuffer* Buffer::CopyToReadOnly(const unsigned char* const from, size_t offset, size_t length)
    {
        return CopyTo(from, offset, length);
    }

    Buffer* Buffer::CopyTo(const Buffer* const from, size_t offset, size_t length)
    {
        return CopyTo(from->start, offset, length);
    }

    Buffer* Buffer::CopyTo(const unsigned char* const from, size_t offset, size_t length)
    {
        uint8_t* newBuff = new uint8_t[length];
        memcopy(from, offset, newBuff, 0, length);

        return new Buffer(newBuff, length);
    }

    Buffer::Buffer()
    {
        //default inits anyways
        length = 0;
        start = nullptr;
    }

    Buffer::Buffer(uint8_t* start, size_t length)
    {
        this->start = start;
        this->length = length;
    }

    Buffer::Buffer(uint8_t* start, size_t length, uint8_t fillWith)
    {
        this->start = start;
        this->length = length;
        memset(start, fillWith, length);
    }

    Buffer::Buffer(size_t reserveSize)
    {
        length = reserveSize;
        start = new uint8_t[length];
    }

    Buffer::~Buffer()
    {
        if (start && owning)
            delete[] start;
        length = 0;
    }

    Buffer::Buffer(const Buffer& copy)
    {
        length = copy.length;
        start = new uint8_t[length];
        memcopy(copy.start, start, length);
    }

    Buffer& Buffer::operator=(const Buffer& copy)
    {
        if (start)
            delete[] start;

        length = copy.length;
        start = new uint8_t[length];
        memcopy(copy.start, start, length);

        return *this;
    }

    Buffer::Buffer(Buffer&& from)
    {
        length = from.length;
        start = from.start;

        from.length = 0;
        from.start = nullptr;
    }

    Buffer& Buffer::operator=(Buffer&& from)
    {
        if (start)
            delete[] start;

        length = from.length;
        start = from.start;

        from.length = 0;
        from.start = nullptr;

        return *this;
    }
    
    void Buffer::Resize(size_t newSize)
    {
        uint8_t* newStart = new uint8_t[newSize];
        size_t copyCount = newSize < length ? newSize : length;
        memcopy(start, newStart, copyCount);

        if (start)
            delete[] start;

        start = newStart;
        length = newSize;
    }

    void Buffer::Set(size_t offset, uint8_t data)
    {
        if (offset >= length)
            return;
        start[offset] = data;
    }

    uint8_t& Buffer::At(size_t offset) const
    {
        if (offset >= length)
            return start[length - 1];
        return start[offset];
    }

    size_t Buffer::Size() const
    {
        return length;
    }

    uint8_t* const Buffer::Data()
    {
        return start;
    }

    const uint8_t* const Buffer::Data() const
    {
        return start;
    }

    uint8_t& Buffer::operator[](size_t index) const
    {
        return At(index);
    }
}