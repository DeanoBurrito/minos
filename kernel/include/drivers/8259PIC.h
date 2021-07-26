#pragma once

#define PORT_PIC1_CMD 0x20
#define PORT_PIC1_DATA 0x21
#define PORT_PIC2_CMD 0xA0
#define PORT_PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01
#define PIC_EOI 0x20

#define PIC1_IDT_OFFSET 0x20
#define PIC2_IDT_OFFSET 0x28

#define PIC_INTERRUPT_PS2KEYBOARD 0x1

namespace Kernel
{
    class PIC
    {
    private:
    public:
        static void Remap();
        static void Disable();

        static void EndMaster();
        static void EndSlave();
    };
}