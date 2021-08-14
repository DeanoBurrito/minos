#pragma once

#include <Buffer.h>

namespace sl
{
    /*  Functions to emulate:
        -snprintf: write to buffer, with limit on written chars.
        -sprintf: write to a buffer
        -fprintf: write to any stream = buffer b; sprintf(b, ...); stream.write(b);
        -printf: write to stdout = fprintf(STD_OUT, ...);
        -vprint/vfprintf/vsprintf/vsnprintf: versions of above using va_list instead of '...'
    */

    /*  Format specifiers: (condensed from https://en.cppreference.com/w/c/io/fprintf)
        NOTE: input stream is ignored until a '%' is found, at which point the stream is passed until specifiers stop being found, and then the requested data is passed.
        '%' = a literal for output '%', assumed to be end of conversion.

        [Flags] = optional
        '-' = conversion is left justified (default is right).
        '+' = forces sign to appear from number conversions.
        ' ' = if signed conversion does not start with a sign, space is prepended. Ignored if '+' is present.
        '#' = perform an alternate conversion (see below).
        '0' = for numbers, pad with zeros instead of spaces. Ignored if '-' is present.

        [Width] = optional
        '*' = width is specified by a integer arg that appears immediately before the argument to be converted.
        *any_number* = minimum number of characters printed by this conversion, padded by spaces.

        [Precision] = optional, starts with a '.'
        *.any_number* = For integers: minimum number of digits written (padded with zeros). 
                        Floating point: digits after decimal point, for g/G number of signicant digits.
                        s specifier: number of characters to print from start unless null reached.
        *.no_number* = assumed to be 0.

        [Length] = optional, if option is incompatable, it is ignored
        *none* = default size is used (int/uint/double)
        'h' = half, means short16, ushort16.
        'hh' = double half, means schar8, uchar8/byte8
        'l' = long, means long64, ulong64 or double (its already double)
        'll' = double long, means the same as above really
        *anything_else* = niche part of the spec, unsupported. 

        [Specifier] = required
        'c' = writes a single character (can be char/int).
        's' = writes a c-string (must be char*). Precision is max chars written if specified.
        'd/i' = writes a signed (decimal/)integer value.
        'o' = writes an unsigned int in octal notation. Alt impl: precision increased as necessary to include the leading 0.
        'x/X' = writes unsigned int in hex. Little x means use little letters, bit x = big letters.
        'u' = writes an unsigned int in decimal representation.
        'f/F' = writes floating point in decimal. Default precision is 6. Alt impl: decimal point printed even if not followed by anything.
        'e/E' = writes floating point in decimal+exponent notation. Exponent is >= 2 digits. Precision is number of characters after decimal point. Alt impl is same as f/F.
        'a/A' = same as e/E but uses hex to encode values.
        'g/G' = use shorted representation of e/E or f/F.
        'n' = returns number of characters written so far by function, argment is int*.
        'p' = implementation defined sequence, argument is void*.
    */
    
    class String;

    //format input to sl::String
    String FormatToString(size_t maxLength, const String* const format, ...);
}