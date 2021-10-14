#include <arch/x86_64/Interrupts.h>
#include <drivers/Ps2Keyboard.h>
#include <drivers/SystemClock.h>
#include <IrqManager.h>
#include <Panic.h>
#include <KLog.h>
#include <StringExtras.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame, uint64_t errorCode)
    {
        Kernel::Panic("Fault Fault x2. (double fault)");
    }
    
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame, uint64_t errorCode)
    {
        Kernel::Log("General Protection fault, error code: 0x", false);
        Kernel::Log(sl::UIntToString(errorCode, BASE_HEX).Data()); //NOTE: this will allocate memory, if its a memory issue, thisll overwrite the error codes we want to see.

        Kernel::Panic("Protection Fault.");
    }

    __attribute__((interrupt)) void PageFault(interrupt_frame* frame, uint64_t errorCode)
    {
        uint64_t faultAddr = 0;
        asm volatile("mov %%cr2, %0": "=g"(faultAddr));
        Kernel::Log("Page fault, addr=0x", false);
        Kernel::Log(sl::UIntToString(faultAddr, BASE_HEX).Data());
        Kernel::Log("Error code: ", false);
        Kernel::Log(sl::UIntToString(errorCode, BASE_HEX).Data());

        Kernel::Panic("Page Fault!");
    }

    __attribute__((interrupt)) void FPUError(interrupt_frame* frame)
    {
        Kernel::Panic("X87 Floating point unit error.");
    }

    __attribute__((interrupt)) void SIMDError(interrupt_frame* frame)
    {
        Kernel::Panic("SIMD/Extended CPU state erorr.");
    }

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame)
    {   
        Kernel::Drivers::Ps2Keyboard::The()->HandlePacketByte();

        Kernel::IrqManager::The()->SendEOI();
    }

    __attribute__((interrupt)) void SystemClockHandler(interrupt_frame* frame)
    {
        Kernel::Drivers::SystemClock::The()->clockTicks++;
        Kernel::IrqManager::The()->SendEOI();
    }
}