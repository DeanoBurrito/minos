#include <StringUtil.h>

#define STATIC_OUTPUT_BUFFER_SIZE 128

char toStr_uint64_buf[STATIC_OUTPUT_BUFFER_SIZE];
const char* ToStr(uint64_t value)
{
    uint8_t size;
    uint64_t sizeTest = value;
    while (sizeTest / 10 > 0)
    {
        sizeTest /= 10;
        size++;
    }

    uint8_t index = 0;
    while (value / 10 > 0)
    {
        uint8_t remainder = value % 10;
        value /= 10;

        toStr_uint64_buf[size - index] = remainder + '0';
        index++;
    }

    uint8_t remainder = value % 10;
    toStr_uint64_buf[size - index] = remainder + '0';
    toStr_uint64_buf[size + 1] = 0;

    return toStr_uint64_buf;
}

const char* ToStr(uint32_t value)
{
    return ToStr(static_cast<uint64_t>(value));
}

const char* ToStr(uint16_t value)
{
    return ToStr(static_cast<uint64_t>(value));
}

const char* ToStr(uint8_t value)
{
    return ToStr(static_cast<uint64_t>(value));
}


char toStr_int64_buf[STATIC_OUTPUT_BUFFER_SIZE];
const char* ToStr(int64_t value)
{
    uint8_t negative = 0;
    if (value < 0)
    {
        negative = 1;
        value *= -1;
        toStr_int64_buf[0] = '-';
    }
    
    uint8_t size;
    uint64_t sizeTest = value;
    while (sizeTest / 10 > 0)
    {
        sizeTest /= 10;
        size++;
    }

    uint8_t index = 0;
    while (value / 10 > 0)
    {
        uint8_t remainder = value % 10;
        value /= 10;

        toStr_int64_buf[negative + size - index] = remainder + '0';
        index++;
    }

    uint8_t remainder = value % 10;
    toStr_int64_buf[negative + size - index] = remainder + '0';
    toStr_int64_buf[negative + size + 1] = 0;

    return toStr_int64_buf;
}

const char* ToStr(int32_t value)
{
    return ToStr(static_cast<int64_t>(value));
}

const char* ToStr(int16_t value)
{
    return ToStr(static_cast<int64_t>(value));
}

const char* ToStr(int8_t value)
{
    return ToStr(static_cast<int64_t>(value));
}


char toStr_double_buf[STATIC_OUTPUT_BUFFER_SIZE];
const char* ToStr(double value, uint8_t decimalPlaces)
{
    if (decimalPlaces > 20)
        decimalPlaces = 0;
    
    char* intPtr = (char*)ToStr((int64_t)value);
    char* doublePtr = toStr_double_buf;

    if (value < 0)
        value *= -1;
    
    while (*intPtr != 0)
    {
        *doublePtr = *intPtr;
        intPtr++;
        doublePtr++;
    }

    if (decimalPlaces == 0)
    {
        *doublePtr = 0;
        return toStr_double_buf;
    }

    *doublePtr = '.';
    doublePtr++;

    double newValue = value - (int)value;

    for (uint8_t i = 0; i < decimalPlaces; i++)
    {
        newValue *= 10;
        *doublePtr = (int)newValue + '0';
        
        newValue -= (int)newValue;
        doublePtr++;
    }

    *doublePtr = 0;
    return toStr_double_buf;
}

char toStr_hex64_buf[STATIC_OUTPUT_BUFFER_SIZE];
const char* ToStrHex(uint64_t value, uint8_t size)
{
    if (size > 15)
        size = 15; //max of 64 bits
    if (size < 1)
        size = 1;
    
    uint64_t *valPtr = &value;
    uint8_t *ptr;
    uint8_t temp;
    //uint8_t size = 8 * 2 - 1;
    for (uint8_t i = 0; i < size; i++)
    {
        ptr = ((uint8_t*)valPtr + i);

        temp = ((*ptr & 0xf0) >> 4);
        toStr_hex64_buf[size - (i * 2 + 1)] = temp + (temp > 9 ? 55 : '0');

        temp = ((*ptr & 0x0f) >> 0);
        toStr_hex64_buf[size - (i * 2)] = temp + (temp > 9 ? 55 : '0');
    }

    toStr_hex64_buf[size + 1] = 0;
    return toStr_hex64_buf;
}

const char* ToStrHex(uint32_t value, uint8_t size)
{
    return ToStrHex(static_cast<uint64_t>(value), size);
}

const char* ToStrHex(uint16_t value, uint8_t size)
{
    return ToStrHex(static_cast<uint64_t>(value), size);
}

const char* ToStrHex(uint8_t value, uint8_t size)
{
    return ToStrHex(static_cast<uint64_t>(value), size);
}
