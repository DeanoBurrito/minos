#pragma once

#include <stdint-gcc.h>

#define IDT_ATTRIBS_InterruptGate 0b10001110
#define IDT_ATTRIBS_CallGate 0b10001100
#define IDT_ATTRIBS_TrapGate 0b10001111

namespace Kernel
{
    struct IDTDescriptorEntry
    {
        uint16_t offset0;
        uint16_t selector;
        uint8_t ist;
        uint8_t attributes;
        uint16_t offset1;
        uint32_t offset2;
        uint32_t reserved;

        void SetOffset(uint64_t offset);
        uint64_t GetOffset();

    } __attribute__((packed));

    struct IDTR
    {
        uint16_t limit;
        uint64_t offset;
    } __attribute__((packed));
}
