#pragma once

#include <stdint.h>

#define IDT_GATE_TYPE_INTERRUPT 0b1110
#define IDT_GATE_TYPE_CALL 0b1100
#define IDT_GATE_TYPE_TRAP 0b1111

namespace Kernel
{
    union IDTEntry
    {
        uint64_t low;
        uint64_t high;

        struct
        {
             uint16_t offset0;
             uint16_t selector;
             uint8_t istNumber; //bits 0-2, rest are zeroed

             union
             {
                uint8_t raw;
                struct
                {
                    uint8_t gateType : 4;
                    uint8_t reserved : 1; //alwways zero
                    uint8_t privilegeLevel : 2; //aka DPL/ring that this executes with
                    uint8_t present : 1;
                } __attribute__((packed));
             } __attribute__((packed)) attributes;

             uint16_t offset1;
             uint32_t offset2;
             uint32_t reserved; //all zeros
        } __attribute__((packed));

        void SetOffset(uint64_t offset);
    } __attribute__((packed));

    struct IDTR
    {
        uint16_t limit;
        uint64_t offset;

        void SetEntry(void* handler, uint64_t vector, uint8_t typesAttribs, uint16_t selector);
    } __attribute__((packed));
}
