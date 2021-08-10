#include <String.h>

namespace Syslib
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

    String::String(const String& copy)
    {
        if (copy.length > 0)
        {
            length = copy.length;
            buffer = new char[length + 1];
            memcopy(copy.buffer, buffer, length + 1);
        }
    }

    String::String(String&& from)
    {
        delete[] buffer;
        length = 0;
        swap(buffer, from.buffer);
        swap(length, from.length);
    }

    String& String::operator=(String&& from)
    {
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
}