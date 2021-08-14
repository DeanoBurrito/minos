#pragma once

#include <stddef.h>
#include <Memory.h>
#include <stdint-gcc.h>

namespace sl
{   
    class String
    {
    private:
        char* buffer;
        size_t length;

    public:
        static String&& FromUInt64(uint64_t unum);
        static String&& FromInt64(int64_t num);

        String();
        String(const char* const cstr);
        String(const char c);
        String(const String& copy);
        String& operator=(const String& copy);
        String(String&& from);
        String& operator=(String&& from);
        ~String();

        const char* const Data() const;
        bool IsEmpty() const;
        size_t Size() const;
        String SubString(size_t start, size_t length) const;

        char& At(size_t index);
        const char& At(size_t index) const;
        char& operator[](size_t index);
        const char& operator[](size_t index) const;

        bool TryGetUInt8(uint8_t& out);
        bool TryGetUInt16(uint16_t& out);
        bool TryGetUInt32(uint32_t& out);
        bool TryGetUInt64(uint64_t& out);
        bool TryGetInt8(int8_t& out);
        bool TryGetInt16(int16_t& out);
        bool TryGetInt32(int32_t& out);
        bool TryGetInt64(int64_t& out);
    };
}

typedef sl::String string;
