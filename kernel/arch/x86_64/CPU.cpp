#include "CPU.h"

namespace Kernel
{
    void CPU::Init()
    {
    }

    void CPU::EnableInterrupts()
    {
        asm volatile("sti");
    }

    void CPU::DisableInterrupts()
    {
        asm volatile("cli");
    }

    void CPU::LoadPageTableMap(void *topLevelAddress)
    {
        asm volatile("mov %0, %%cr3"
                     :
                     : "r"(topLevelAddress));
    }

    extern "C" 
    {
        extern void LoadGDT_impl(GDTDescriptor* address);
    }
    
    void CPU::LoadGDT(GDTDescriptor* address)
    {
        LoadGDT_impl(address);
    }

    void CPU::LoadIDT(IDTR *idtr)
    {
        asm volatile("lidt 0(%0)"
                     :
                     : "r"(idtr));
    }

    void CPU::Halt()
    {
        asm volatile("     \
    loop: \n            \
        hlt\n           \
        jmp loop\n      \
    ");
    }

    void CPU::PortWrite8(uint16_t port, uint8_t data)
    {
        asm volatile("outb %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    void CPU::PortWrite16(uint16_t port, uint16_t data)
    {
        asm volatile("outw %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    void CPU::PortWrite32(uint16_t port, uint32_t data)
    {
        asm volatile("outl %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    uint8_t CPU::PortRead8(uint16_t port)
    {
        uint8_t value;
        asm volatile("inb %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    uint16_t CPU::PortRead16(uint16_t port)
    {
        uint16_t value;
        asm volatile("inw %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    uint32_t CPU::PortRead32(uint16_t port)
    {
        uint32_t value;
        asm volatile("inl %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    void CPU::PortIOWait()
    {
        //read from port 0x80 (will force io lines to complete previous operations)
        PortRead8(PORT_USUALLY_EMPTY);
    }

    char cpuArch[] = "x86_64";
    char *CPU::GetArchitectureName()
    {
        return cpuArch;
    }
}
