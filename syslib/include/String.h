#pragma once

#include <Memory.h>

namespace Syslib
{   
    class String
    {
    private:
        char* buffer;
        size_t length;

    public:
        String();
        String(const char* const cstr);
        String(const String& copy);
        String& operator=(const String& copy);
        String(String&& from);
        String& operator=(String&& from);
        ~String();

        const char* const Data() const;
        bool IsEmpty() const;
        size_t Size() const;

    };
}

typedef Syslib::String string;
