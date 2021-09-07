#include <Interrupts.h>
#include <Panic.h>
#include <drivers/CPU.h>
#include <drivers/Ps2Keyboard.h>
#include <drivers/APIC.h>
#include <drivers/8253PIT.h>
#include <KLog.h>
#include <StringExtras.h>

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame, uint64_t errorCode)
    {
        Kernel::Panic("Fault Fault x2. (double fault)");

        Kernel::Drivers::CPU::Halt();
    }
    
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame, uint64_t errorCode)
    {
        Kernel::Log("General Protection fault, error code: 0x", false);
        Kernel::Log(sl::UIntToString(errorCode, BASE_HEX).Data()); //NOTE: this will allocate memory, if its a memory issue, thisll overwrite the error codes we want to see.

        Kernel::Panic("Protection Fault.");
        Kernel::Drivers::CPU::Halt();
    }

    __attribute__((interrupt)) void PageFault(interrupt_frame* frame, uint64_t errorCode)
    {
        Kernel::Panic("Page Fault!"); //CR2 contains our culprit

        Kernel::Drivers::CPU::Halt();
    }

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame)
    {   
        uint8_t scancode = Kernel::Drivers::CPU::PortRead8(PORT_PS2_KEYBOARD);
        Kernel::Drivers::Ps2Keyboard::The()->HandlePacketByte(scancode);

        Kernel::Drivers::APIC::Local()->SendEOI();
    }

    __attribute__((interrupt)) void DefaultTimerHandler(interrupt_frame* frame)
    {
        Kernel::Drivers::PIT::ticks++;

        Kernel::Drivers::APIC::Local()->SendEOI();
    }
}