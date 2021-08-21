#pragma once

#include <stddef.h>
#ifdef __cplusplus
#include <stdint-gcc.h>
#else
#include <stdint.h>
#endif
#include "PSF1.h"

#define PIXEL_FORMAT_Unknown 0
#define PIXEL_FORMAT_RedGreenBlueReserved_8BPP 1
#define PIXEL_FORMAT_BlueGreenRedReserved_8BPP 2

#define MEMORY_DESCRIPTOR_RESERVED 0x0
#define MEMORY_DESCRIPTOR_FREE 0x1

typedef struct
{
    uint64_t physicalStart;
    uint64_t virtualStart;
    uint64_t numberOfPages;
    uint64_t flags;
} MemoryRegionDescriptor;

typedef struct
{
    struct GOP
    {
        void *baseAddress;
        size_t bufferSize;
        unsigned int width;
        unsigned int height;
        unsigned int pixelsPerScanline;
        unsigned int pixelFormat;
    } gop;

    PSF1_Font *font;

    uint64_t memoryDescriptorsCount;
    MemoryRegionDescriptor* memoryDescriptors;

    void* rsdp;
} BootInfo;
