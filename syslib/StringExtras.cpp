#include <StringExtras.h>

namespace sl
{
    //TODO: move all these functions into a 'locale/language-sensitive container', so definitions can be swapped out as needed. Default is ASCII

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
    { return c; }

    SL_CHAR_TYPE GetUpper(const SL_CHAR_TYPE& c)
    { return c; }

}