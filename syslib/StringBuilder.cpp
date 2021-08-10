#include <StringBuilder.h>
#include <Memory.h>

namespace Syslib
{
    StringBuilder StringBuilder::FromFormatString(string str, char delim)
    {
        return StringBuilder();
    }

    StringBuilder::StringBuilder()
    {}

    StringBuilder::StringBuilder(string str)
    {
        textLength = str.Size();
        Buffer* convertedString = Buffer::CopyTo(reinterpret_cast<const unsigned char* const>(str.Data()), 0, str.Size());
        buffers.PushFront(move(*convertedString)); //move from temp buffer and then delete it
        delete convertedString;
    }

    StringBuilder::~StringBuilder()
    {}

    StringBuilder::StringBuilder(StringBuilder&& from)
    {
        buffers.Clear();
        swap(buffers, from.buffers);
        textLength = 0;
        swap(textLength, from.textLength);
    }

    StringBuilder& StringBuilder::operator=(StringBuilder&& from)
    {
        buffers.Clear();
        swap(buffers, from.buffers);
        textLength = 0;
        swap(textLength, from.textLength);

        return *this;
    }

    string StringBuilder::ToString()
    {
        Buffer outputBuffer(textLength + 1);
        size_t currentBase = 0;

        auto scan = buffers.Head();
        while (scan != nullptr)
        {
            memcopy(scan->val.Data(), 0, outputBuffer.Data(), currentBase, scan->val.Size());
            currentBase += scan->val.Size();

            scan = scan->next;
        }

        outputBuffer[textLength] = 0;
        return String(reinterpret_cast<const char* const>(outputBuffer.Data()));
    }

    size_t StringBuilder::Size()
    {
        return textLength;
    }

    void StringBuilder::Clear()
    {
        buffers.Clear();
        textLength = 0;
    }

    Buffer& StringBuilder::BufferFromIndex(size_t& index)
    {
        if (index >= textLength)
            return buffers.Tail()->val;
        
        auto scan = buffers.Head();
        while (scan != nullptr)
        {
            if (scan->val.Size() > index)
                return scan->val;

            index -= scan->val.Size();
            scan = scan->next;
        }

        return buffers.Tail()->val;
    }

    char& StringBuilder::operator[](size_t index)
    {
        Buffer buff = BufferFromIndex(index);
        return reinterpret_cast<char&>(buff[index]); 
    }
}