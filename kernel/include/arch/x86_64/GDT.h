#pragma once

#include <stdint.h>

namespace Kernel
{
    struct GDTDescriptor
    {
        uint16_t size;
        uint64_t offset;
    } __attribute__((packed));

    struct GDTEntry
    {
        uint16_t limit0;
        uint16_t base0;
        uint8_t base1;
        uint8_t access;
        uint8_t limit1_flags;
        uint8_t base2;
    } __attribute__((packed));

    struct GDT
    {
        GDTEntry null;       //0x00
        GDTEntry kernelCode; //0x08
        GDTEntry kernelData; //0x10
        GDTEntry userNull;   //0x12
        GDTEntry userCode;   //0x20
        GDTEntry userData;   //0x
    } __attribute__((packed)) __attribute((aligned(4096)));

    extern GDT defaultGdt;
}
