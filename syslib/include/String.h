#pragma once

#include <stddef.h>
#include <Memory.h>
#include <stdint-gcc.h>
#include <Limits.h>
#include <templates/List.h>

#define TOKEN_NOT_FOUND UINT64_UPPER_LIMIT

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

        size_t Find(const char token, size_t offset = 0) const;
        List<String> Split(const char token) const;

        char& At(size_t index);
        const char& At(size_t index) const;
        char& operator[](size_t index);
        const char& operator[](size_t index) const;
    };
}

typedef sl::String string;
