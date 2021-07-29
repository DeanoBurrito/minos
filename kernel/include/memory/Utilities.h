#pragma once

#include <stdint-gcc.h>

void memset(void* const start, uint8_t value, uint64_t count);
void memcopy(const void* const source, void* const destination, uint64_t count);
int memcmp(const void* const  a, const void* const b, uint64_t count);