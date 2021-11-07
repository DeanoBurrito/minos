#pragma once

#include <collections/LinkedList.h>
#include <Buffer.h>
#include <String.h>

namespace sl
{
    class StringBuilder
    {
    private:
        LinkedList<Buffer> buffers;
        size_t textLength;
        Buffer& BufferFromIndex(size_t& index);

    public:
        StringBuilder();
        StringBuilder(string str);

        ~StringBuilder();
        StringBuilder(const StringBuilder& copy) = delete;
        StringBuilder& operator=(const StringBuilder& copy) = delete;
        StringBuilder(StringBuilder&& from);
        StringBuilder& operator=(StringBuilder&& from);

        void Append(const string& str);

        string ToString();
        size_t Size();
        void Clear();
        char& operator[](size_t index);
    };
}