#pragma once

#include <stdint-gcc.h>

void memset(void* start, uint8_t value, uint64_t count);
void memcopy(void* source, void* destination, uint64_t count);