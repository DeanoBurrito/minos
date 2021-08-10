#include <drivers/CPU.h>
#include <drivers/8259PIC.h>

namespace Kernel::Drivers
{   
    void PIC::Remap()
    {
        //store previous maps of master and slave chips
        uint8_t pic1Mask = CPU::PortRead8(PORT_PIC1_DATA);
        CPU::PortIOWait();
        uint8_t pic2Mask = CPU::PortRead8(PORT_PIC2_DATA);
        CPU::PortIOWait();

        //program master PIC
        CPU::PortWrite8(PORT_PIC1_CMD, ICW1_INIT | ICW1_ICW4);
        CPU::PortIOWait();
        //program slave PIC
        CPU::PortWrite8(PORT_PIC2_CMD, ICW1_INIT | ICW1_ICW4);
        CPU::PortIOWait();

        //set offsets for both chips
        CPU::PortWrite8(PORT_PIC1_DATA, PIC1_IDT_OFFSET);
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, PIC2_IDT_OFFSET);
        CPU::PortIOWait();

        //set master and slave status on PICs
        CPU::PortWrite8(PORT_PIC1_DATA, 4);
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, 2);
        CPU::PortIOWait();

        //set operation mode to 8086 mode, rather than even more legacy mode.
        CPU::PortWrite8(PORT_PIC1_DATA, ICW4_8086);
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, ICW4_8086);
        CPU::PortIOWait();

        //restore saved interrupt masks
        CPU::PortWrite8(PORT_PIC1_DATA, pic1Mask);
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, pic2Mask);
        CPU::PortIOWait();
    }

    void PIC::Disable()
    {
        CPU::PortWrite8(PORT_PIC1_DATA, 0xFF);
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, 0xFF);
        CPU::PortIOWait();
    }

    void PIC::EndMaster()
    {
        CPU::PortWrite8(PORT_PIC1_CMD, PIC_EOI);
    }

    void PIC::EndSlave()
    {
        CPU::PortWrite8(PORT_PIC2_CMD, PIC_EOI);
        CPU::PortWrite8(PORT_PIC1_CMD, PIC_EOI);
    }
}