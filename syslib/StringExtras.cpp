#include <StringExtras.h>

namespace sl
{
    //TODO: move all these functions into a 'locale/culture/language-sensitive container', so definitions can be swapped out as needed. Default is ASCII

    bool IsAlpha(const SL_CHAR_TYPE& c)
    { 
        if (c >= 'a' && c <= 'z')
            return true;
        if (c >= 'A' && c <= 'Z')
            return true;
        
        return false;
    }

    bool IsWhitespace(const SL_CHAR_TYPE& c)
    { 
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            return true;
        
        return false;
    }

    bool IsPrintable(const SL_CHAR_TYPE& c)
    { 
        //TODO: implement in a nice way
        return false;
    }

    bool IsDigit(const SL_CHAR_TYPE& c)
    { 
        if (c >= '0' && c <= '9')
            return true;

        return false;
    }

    bool IsHexDigit(const SL_CHAR_TYPE& c)
    { 
        if (c >= '0' && c <= '9')
            return true;
        if (c >= 'A' && c <= 'F')
            return true;
        if (c >= 'a' && c <= 'f')
            return true;
        
        return false;
    }

    bool IsUppercase(const SL_CHAR_TYPE& c)
    { 
        if (c >= 'A' && c <= 'Z')
            return true;

        return false;
    }

    bool IsLower(const SL_CHAR_TYPE& c)
    { 
        if (c >= 'a' && c <= 'z')
            return true;
        
        return false;
    }

    SL_CHAR_TYPE GetLower(const SL_CHAR_TYPE& c)
    {
        if (c >= 'A' && c <= 'Z')
            return c + 0x20;
        return c;
    }

    SL_CHAR_TYPE GetUpper(const SL_CHAR_TYPE& c)
    { 
        if (c >= 'a' && c <= 'z')
            return c - 0x20;
        return c;
    }

    string UIntToString(uint64_t num, int base)
    {
        if (base < 2 || base > 32)
            return "";
        
        char* buffer = new char[21];
        size_t index = 0;

        while (num)
        {
            int remainder = num % base;
            num = num / base;

            if (remainder < 10)
                buffer[index] = '0' + remainder;
            else
                buffer[index] = 'A' + remainder - 10;
            index++;
        }

        if (index == 0 && num == 0)
        {
            buffer[index] = '0';
            index++;
        }

        buffer[index] = 0;
        char temp;
        for (size_t i = 0; i < index / 2; i++)
        {
            temp = buffer[i];
            buffer[i] = buffer[index - i - 1];
            buffer[index - i - 1] = temp;
        }

        string str(buffer);
        delete[] buffer;
        return str;
    }

    string IntToString(int64_t num, int base)
    {
        if (base < 2 || base > 32)
            return "";
        
        char* buffer = new char[21];
        size_t index = 0;
        bool appendMinus = false;

        if (num < 0)
        {
            appendMinus = true;
            num = -num;
        }

        while (num)
        {
            int remainder = num % base;
            num = num / base;

            if (remainder < 10)
                buffer[index] = '0' + remainder;
            else
                buffer[index] = 'A' + remainder - 10;
            index++;
        }

        if (index == 0 && num == 0)
        {
            buffer[index] = '0';
            index++;
        }

        if (appendMinus)
        {
            buffer[index] = '-';
            index++;
        }

        buffer[index] = 0;
        char temp;
        for (size_t i = 0; i < index / 2; i++)
        {
            temp = buffer[i];
            buffer[i] = buffer[index - i - 1];
            buffer[index - i - 1] = temp;
        }

        string str(buffer);
        delete[] buffer;
        return str;

        return "";
    }

    string DoubleToString(double num, int base, size_t precision)
    {
        //TODO: implement this once we have FPU support. Great resource below.
        //https://git.musl-libc.org/cgit/musl/blob/src/stdio/vfprintf.c?h=v1.2.2

        return "";
    }

    bool TryGetUInt8(const string& from, uint8_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetUInt16(const string& from, uint16_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetUInt32(const string& from, uint32_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetUInt64(const string& from, uint64_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetInt8(const string& from, int8_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetInt16(const string& from, int16_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetInt32(const string& from, int32_t& out, size_t offset)
    {
        return false;
    }

    bool TryGetInt64(const string& from, int64_t& out, size_t offset)
    {
        return false;
    }
}