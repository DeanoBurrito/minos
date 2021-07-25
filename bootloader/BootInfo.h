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

typedef struct
{
    uint64_t type;
    uint64_t physicalStart;
    uint64_t virtualStart;
    uint64_t numberOfPages;
    uint64_t attribute;
} EfiMemoryDescriptor;

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

    struct EfiMemoryMap
    {
        EfiMemoryDescriptor *descriptor;

        uint64_t size;
        uint64_t descriptorSize;
        uint64_t key;
        uint32_t descriptorVersion;
    } memoryMap;
} BootInfo;
