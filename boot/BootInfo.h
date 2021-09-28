#pragma once

#include <stdint.h>

#define PIXEL_FORMAT_Unknown 0
#define PIXEL_FORMAT_RedGreenBlueReserved_8BPP 1
#define PIXEL_FORMAT_BlueGreenRedReserved_8BPP 2

/*
    NOTE: I want everything here to be explicit in it's definition, as the bootloader/kernel could theoritically be compiled on separate platforms.
    However unlikely, its not impossible, hence the use of explicitly sized integers. No size_t's or pointers depending on machine word size.
*/

typedef uint64_t NativePtr;

typedef struct
{
    NativePtr physicalStart;
    NativePtr virtualStart;
    uint64_t numberOfPages;
    
    union
    {
        uint64_t raw;
        struct
        {
            uint8_t free : 1;
            uint8_t mustMap : 1;
        };
    } flags;
} MemoryRegionDescriptor __attribute__((aligned(8)));

typedef struct
{
    NativePtr base;
    uint64_t bufferSize;
    uint64_t width;
    uint64_t height;
    uint64_t stride;
    uint64_t pixelFormat;
} BootFramebuffer __attribute__((aligned(8)));

typedef struct
{
    BootFramebuffer framebuffer;

    uint64_t memoryDescriptorsCount;
    MemoryRegionDescriptor* memoryDescriptors;

    NativePtr rsdp;

    NativePtr kernelStartAddr;
    uint64_t kernelSize; 
} BootInfo __attribute__((aligned(8)));
