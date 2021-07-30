#include <Interrupts.h>
#include <Serial.h>
#include <Panic.h>
#include <CPU.h>
#include <drivers/Ps2Keyboard.h>
#include <drivers/APIC.h>

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame)
    {
        Kernel::Panic("Fault Fault x2. (double fault)");

        Kernel::CPU::Halt();
    }
    
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame)
    {
        Kernel::Panic("Protection Fault.");

        Kernel::CPU::Halt();
    }

    __attribute__((interrupt)) void PageFault(interrupt_frame* frame)
    {
        Kernel::Panic("Page Fault!"); //CR2 contains our culprit

        Kernel::CPU::Halt();
    }

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame)
    {   
        uint8_t scancode = Kernel::CPU::PortRead8(PORT_PS2_KEYBOARD);
        Kernel::Drivers::Ps2Keyboard::The()->HandlePacketByte(scancode);

        Kernel::Drivers::APIC::Local()->SendEOI();
    }

    __attribute__((interrupt)) void TimerHandler(interrupt_frame* frame)
    {
        Kernel::Drivers::APIC::Local()->SendEOI();
    }
}