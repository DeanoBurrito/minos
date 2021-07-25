#pragma once

#include <stdint-gcc.h> 

const char* ToStr(uint64_t value);
const char* ToStr(int64_t value);
const char* ToStr(double value, uint8_t decimalPlaces = 2);

const char* ToStrHex(uint64_t value, uint8_t size = 15);