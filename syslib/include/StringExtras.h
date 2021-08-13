#pragma once

//to make life easier when supporting different text encodings (utf8)
#define SL_CHAR_TYPE char

namespace sl
{
    //returns true is a character is alphabetic
    bool IsAlpha(const SL_CHAR_TYPE& c);
    //bool IsChad(); //This is a such a dumb joke, I cant believe i made this.
    //returns true if a character is whitespace
    bool IsWhitespace(const SL_CHAR_TYPE& c);
    //returns true character can be printed
    bool IsPrintable(const SL_CHAR_TYPE& c);
    //returns if a character is a number
    bool IsDigit(const SL_CHAR_TYPE& c);
    //returns if a character is a hexadecimal number
    bool IsHexDigit(const SL_CHAR_TYPE& c);
    //returns if a character is uppercase (if supported)
    bool IsUppercase(const SL_CHAR_TYPE& c);
    //returns if a character is lowercase (if supported)
    bool IsLower(const SL_CHAR_TYPE& c);

    //gets the lowercase representation of a character (if available)
    SL_CHAR_TYPE GetLower(const SL_CHAR_TYPE& c);
    //gets the uppercase representation of character (if available)
    SL_CHAR_TYPE GetUpper(const SL_CHAR_TYPE& c);
}