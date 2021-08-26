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

    String String::SubString(size_t start, size_t len) const
    {
        if (start + len > length)
            len = length - start;

        //TODO: double copy here, would be nice to be able to move a char[] to a string.
        char* const tempBuffer = new char[len + 1];
        sl::memcopy(buffer, start, tempBuffer, 0, len);
        tempBuffer[len] = 0;
        String temp(tempBuffer);
        delete[] tempBuffer;

        return temp;
    }

    size_t String::Find(const char token, size_t offset) const
    {
        size_t where = memfirst(buffer, offset, token, length);
        
        if (where > 0)
            return where;
        if (buffer[0] == token)
            return 0;
        return TOKEN_NOT_FOUND;
    }

    List<String> String::Split(const char token) const
    {
        size_t segmentStart = 0;
        size_t segmentLength = 0;
        List<String> list;

        for (int i = 0; i < length; i++)
        {
            if (At(i) == token)
            {
                if (segmentLength > 0)
                    list.PushBack(move(SubString(segmentStart, segmentLength)));
                
                segmentStart = i + 1; //we dont want to prepend the next word with the delim
                segmentLength = 0;
            }
            else
                segmentLength++;
        }

        if (segmentLength > 0)
            list.PushBack(move(SubString(segmentStart, segmentLength)));

        return list;
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
}