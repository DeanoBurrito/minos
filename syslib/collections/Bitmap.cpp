#include <collections/Bitmap.h>
#include <Memory.h>
#include <Maths.h>

namespace sl
{
    Bitmap::Bitmap()
    {
        buffer = nullptr;
        bufferLength = 0;
    }

    Bitmap::Bitmap(size_t count)
    {
        bufferLength = (count / 8) + 1; //convert bits to bytes, and round up.
        buffer = new uint8_t[bufferLength];
        sl::memset(buffer, 0, bufferLength);
    }

    Bitmap::~Bitmap()
    {
        if (buffer)
            delete[] buffer;
        bufferLength = 0;
    }

    Bitmap::Bitmap(const Bitmap& other)
    {
        buffer = nullptr;
        bufferLength = 0;

        if (other.buffer)
        {
            buffer = new uint8_t[other.bufferLength];
            bufferLength = other.bufferLength;
            sl::memcopy(other.buffer, buffer, bufferLength);
        }
    }
    
    Bitmap& Bitmap::operator=(const Bitmap& other)
    {
        if (buffer)
        {
            delete[] buffer;
            bufferLength = 0;
        }

        if (other.buffer)
        {
            buffer = new uint8_t[other.bufferLength];
            bufferLength = other.bufferLength;
            sl::memcopy(other.buffer, buffer, bufferLength);
        }

        return *this;
    }
    
    Bitmap::Bitmap(Bitmap&& from)
    {
        sl::swap(from.buffer, buffer);
        sl::swap(from.bufferLength, bufferLength);
    }
    
    Bitmap& Bitmap::operator=(Bitmap&& from)
    {
        sl::swap(from.buffer, buffer);
        sl::swap(from.bufferLength, bufferLength);
        return *this;
    }

    void Bitmap::Resize(size_t newCount)
    {
        size_t prevLength = bufferLength;
        bufferLength = (newCount / 8) + 1;

        uint8_t* newBuffer = new uint8_t[bufferLength];
        sl::memset(newBuffer, 0, bufferLength);
        
        size_t copylength = Min(prevLength, bufferLength);
        sl::memcopy(buffer, newBuffer, copylength);

        delete[] buffer;
        buffer = newBuffer;
    }

    bool Bitmap::Get(size_t index)
    {
        size_t byteIndex = index / 8;
        index = index % 8;
        
        if (byteIndex >= bufferLength)
            return false;
        
        return (buffer[byteIndex] & (1 << index)) != 0;
    }

    void Bitmap::Set(size_t index, bool status)
    {
        size_t byteIndex = index / 8;
        index = index % 8;

        if (byteIndex >= bufferLength)
            return;
        
        uint8_t temp = buffer[byteIndex];
        if (status)
            temp |= (1 << index);
        else
            temp &= ~(1 << index);
        buffer[byteIndex] = temp;
    }

    void Bitmap::Clear(size_t index)
    {
        size_t byteIndex = index / 8;
        index = index % 8;

        if (byteIndex >= bufferLength)
            return;
    }
}
