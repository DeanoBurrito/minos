#pragma once

#include <stdint-gcc.h> 

const char* ToStr(uint64_t value);
const char* ToStr(uint32_t value);
const char* ToStr(uint16_t value);
const char* ToStr(uint8_t value);

const char* ToStr(int64_t value);
const char* ToStr(int32_t value);
const char* ToStr(int16_t value);
const char* ToStr(int8_t value);

const char* ToStr(double value, uint8_t decimalPlaces = 2);

const char* ToStrHex(uint64_t value, uint8_t size = 15);
const char* ToStrHex(uint32_t value, uint8_t size = 7);
const char* ToStrHex(uint16_t value, uint8_t size = 3);
const char* ToStrHex(uint8_t value, uint8_t size = 1);

unsigned int strlen(const char* const str);