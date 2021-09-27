#pragma once

#include <stdint.h>

#define PIXEL_FORMAT_Unknown 0
#define PIXEL_FORMAT_RedGreenBlueReserved_8BPP 1
#define PIXEL_FORMAT_BlueGreenRedReserved_8BPP 2

#define MEMORY_DESCRIPTOR_RESERVED 0x0
#define MEMORY_DESCRIPTOR_FREE 0x1

/*
    NOTE: all of these structs should be using explicitly sized integers, no size_t's here. Since bootloaders and kernel
    can be compiled at different times, we have no idea (they should) if they're using the same stddefs.
    Everything is also 8-byte aligned (64bit wordsize).
*/

typedef struct
{
    uint64_t physicalStart;
    uint64_t virtualStart;
    uint64_t numberOfPages;
    uint64_t flags;
} MemoryRegionDescriptor __attribute__((aligned(8)));

typedef struct
{
    uint64_t base;
    uint64_t bufferSize;
    uint64_t width;
    uint64_t height;
    uint64_t stride;
    uint64_t pixelFormat;
} BootFramebuffer __attribute__((aligned(8)));

typedef struct
{
    struct GOP
    {
        void *baseAddress;
        uint64_t bufferSize;
        unsigned int width;
        unsigned int height;
        unsigned int pixelsPerScanline;
        unsigned int pixelFormat;
    } gop;

    uint64_t memoryDescriptorsCount;
    MemoryRegionDescriptor* memoryDescriptors;

    void* rsdp;
} BootInfo __attribute__((aligned(8)));
