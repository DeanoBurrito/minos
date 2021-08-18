#pragma once

#include <stdint-gcc.h>
#include <String.h>

//to make life easier when supporting different text encodings (utf8)
#define SL_CHAR_TYPE char
#define BASE_BINRAY 2
#define BASE_DECIMAL 10
#define BASE_HEX 16

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

    string UIntToString(uint64_t num, int base);
    string IntToString(int64_t num, int base);
    string DoubleToString(double num, int base, size_t precision);

    bool TryGetUInt8(const string& from, uint8_t& out, size_t offset = 0);
    bool TryGetUInt16(const string& from, uint16_t& out, size_t offset = 0);
    bool TryGetUInt32(const string& from, uint32_t& out, size_t offset = 0);
    bool TryGetUInt64(const string& from, uint64_t& out, size_t offset = 0);
    bool TryGetInt8(const string& from, int8_t& out, size_t offset = 0);
    bool TryGetInt16(const string& from, int16_t& out, size_t offset = 0);
    bool TryGetInt32(const string& from, int32_t& out, size_t offset = 0);
    bool TryGetInt64(const string& from, int64_t& out, size_t offset = 0);
}