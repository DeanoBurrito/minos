#include "Interrupts.h"
#include "Serial.h"
#include "Panic.h"
#include "KRenderer.h"
#include "CPU.h"
#include "PIC.h"

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
        Kernel::KRenderer::The()->Write("KeyPressed! ");
        uint8_t scancode = Kernel::CPU::PortRead8(0x60);

        Kernel::PIC::EndMaster();
    }
}