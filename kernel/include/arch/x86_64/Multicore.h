#pragma once

#include <Platform.h>
#include <arch/x86_64/GDT.h>
#include <arch/x86_64/IDT.h>
#include <PageTableManager.h>
#include <drivers/APIC.h>
#include <drivers/X86Extensions.h>
#include <multiprocessing/Scheduler.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64);

namespace Kernel
{
    enum class CoreOperationState : uint8_t
    {
        Normal,
    };
    
    struct CoreLocalStorage
    {
        uint64_t mailboxSize;
        uint64_t mailboxAddr;

        uint64_t workingStack;
        CoreOperationState opState;

        IDTR idt;
        GDT gdt;
        Drivers::APIC apic;
        Drivers::X86Extensions cpuExtensions;
        PageTableManager pageTableManager;
        //TODO: PMM dispatch/sync
        Multiprocessing::Scheduler scheduler;
    };
}
