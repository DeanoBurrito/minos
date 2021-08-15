#include <Memory.h>
#include <String.h>

namespace sl
{
    String::String() : buffer(nullptr)
    {}

    String::String(const char* const cstr)
    {
        length = memfirst(cstr, 0, 0);
        buffer = new char[length + 1];
        memcopy(cstr, buffer, length);
        buffer[length] = 0;
    }

    String::String(const char c)
    {
        length = 1;
        buffer = new char[2];
        buffer[0] = c;
        buffer[1] = 0;
    }

    String::String(const String& copy)
    {
        if (copy.length > 0)
        {
            length = copy.length;
            buffer = new char[length + 1];
            memcopy(copy.buffer, buffer, length);
            buffer[length] = 0;
        }
    }

    String& String::operator=(const String& copy)
    {
        if (buffer)
            delete[] buffer;

        length = copy.length;
        buffer = new char[length + 1];
        memcopy(copy.buffer, buffer, length);
        buffer[length] = 0;

        return *this;
    }

    String::String(String&& from)
    {
        buffer = nullptr;
        length = 0;
        swap(buffer, from.buffer);
        swap(length, from.length);
    }

    String& String::operator=(String&& from)
    {
        if (buffer)
            delete[] buffer;
        length = 0;
        swap(buffer, from.buffer);
        swap(length, from.length);

        return *this;
    }

    String::~String()
    {
        if (buffer)
            delete[] buffer;
        length = 0;
    }

    const char* const String::Data() const
    {
        return buffer;
    }

    bool String::IsEmpty() const
    {
        return length > 0;
    }

    size_t String::Size() const
    {
        return length;
    }

    String String::SubString(size_t start, size_t length) const
    {
        if (start + length > this->length)
            length = this->length - start;

        //TODO: double copy here, would be nice to be able to move a char[] to a string.
        char* const tempBuffer = new char[length + 1];
        sl::memcopy(buffer, start, tempBuffer, 0, length);
        tempBuffer[length] = 0;
        String temp(tempBuffer);
        delete[] tempBuffer;

        return temp;
    }

    char& String::At(size_t index)
    {
        if (index >= length)
            return buffer[length - 1];
        return buffer[index];
    }

    const char& String::At(size_t index) const
    {
        if (index >= length)
            return buffer[length - 1];
        return buffer[index];
    }

    char& String::operator[](size_t index)
    {
        return At(index);
    }

    const char& String::operator[](size_t index) const
    {
        return At(index);
    }

    bool String::TryGetUInt8(uint8_t& out)
    { return true; }

    bool String::TryGetUInt16(uint16_t& out)
    { return true; }

    bool String::TryGetUInt32(uint32_t& out)
    { return true; }

    bool String::TryGetUInt64(uint64_t& out)
    { return true; }

    bool String::TryGetInt8(int8_t& out)
    { return true; }

    bool String::TryGetInt16(int16_t& out)
    { return true; }

    bool String::TryGetInt32(int32_t& out)
    { return true; }

    bool String::TryGetInt64(int64_t& out)
    { return true; }

}