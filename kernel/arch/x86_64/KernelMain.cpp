#include <BootInfo.h>
#include <KRenderer.h>
#include <PageFrameAllocator.h>
#include <PageTableManager.h>
#include <Memory.h>
#include <drivers/CPU.h>
#include <arch/x86_64//IDT.h>
#include <arch/x86_64/GDT.h>
#include <arch/x86_64/Interrupts.h>
#include <drivers/Serial.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Panic.h>
#include <memory/KHeap.h>
#include <drivers/ACPI.h>
#include <drivers/APIC.h>
#include <drivers/HPET.h>
#include <drivers/8253PIT.h>
#include <drivers/X86Extensions.h>
#include <multiprocessing/Scheduler.h>
#include <kshell/KShell.h>
#include <Formatting.h>
#include <InitDisk.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

extern "C"
{
extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;
extern void _init();
}

namespace Kernel
{
    using namespace Kernel::Drivers;

    void InitMemory(BootInfo* bootInfo)
    {        
        //assemble memory map into something we can use.
        PageFrameAllocator::The()->Init(bootInfo);

        //make sure kernel and framebuffer have their physical pages reserved.
        uint64_t kernelSizePages = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
        kernelSizePages = kernelSizePages / PAGE_SIZE + 1; //round up to nearest page size
        PageFrameAllocator::The()->ReservePages((void*)bootInfo->kernelStartAddr, kernelSizePages);
        
        uint64_t framebufferSizePages = bootInfo->framebuffer.bufferSize / PAGE_SIZE + 1;
        PageFrameAllocator::The()->ReservePages((void*)bootInfo->framebuffer.base, framebufferSizePages);

        //initialize virtual memory
        PageTableManager::The()->Init();

        //identity map kernel and framebuffer
        uint64_t framebufferEnd = bootInfo->framebuffer.base + bootInfo->framebuffer.bufferSize;
        for (sl::UIntPtr kernelPtr = bootInfo->kernelStartAddr; kernelPtr.raw < (uint64_t)&_KernelEnd; kernelPtr.raw += PAGE_SIZE)
            PageTableManager::The()->MapMemory(kernelPtr.ptr, kernelPtr.ptr, MemoryMapFlags::WriteAllow | MemoryMapFlags::ExecuteAllow);
        for (sl::UIntPtr fbPtr = bootInfo->framebuffer.base; fbPtr.raw < framebufferEnd; fbPtr.raw += PAGE_SIZE)
            PageTableManager::The()->MapMemory(fbPtr.ptr, fbPtr.ptr, MemoryMapFlags::WriteAllow);

        //identity map any regions requested by pre-kernel
        MemoryRegionDescriptor* memRegion = bootInfo->memoryDescriptors;
        for (size_t i = 0; i < bootInfo->memoryDescriptorsCount; i++)
        {
            if (memRegion->flags.mustMap)
            {
                for (size_t j = 0; j < memRegion->numberOfPages; j++)
                    PageTableManager::The()->MapMemory(
                        (void*)(memRegion->virtualStart + (j * PAGE_SIZE)), 
                        (void*)(memRegion->physicalStart + (j * PAGE_SIZE)), 
                        MemoryMapFlags::WriteAllow | MemoryMapFlags::ExecuteAllow);
            }

            memRegion++;
        }

        //---- THE FINAL HURDLE
        //identity map all pages for now. TODO: only map what we need
        for (uint64_t i = 0; i < PageFrameAllocator::The()->GetTotalMemory(); i += PAGE_SIZE)
            PageTableManager::The()->MapMemory((void*)i, (void*)i, MemoryMapFlags::WriteAllow);
        //----

        //switch to our own virtual memory map
        PageTableManager::The()->MakeCurrent();

        //initialize kernel heap at an arbitrarily large address
        KHeap::The()->Init((void*)0x100'000'000, PAGE_SIZE);
    }

    GDTDescriptor defaultGdtDescriptor;
    void InitPlatformEarly(BootInfo* bootInfo)
    {
        //load our default gdt
        defaultGdtDescriptor.size = sizeof(GDT) - 1;
        defaultGdtDescriptor.offset = (uint64_t)&defaultGdt;
        CPU::LoadTable(CpuTable::x86_64_GDT, &defaultGdtDescriptor);
        
        //call global constructors
        _init();

        //get serial logging up as early as we can
        InitLogging();
        SerialPort::COM1()->Init(PORT_COM1_ADDRESS);
        SetSerialLogging(true);
        Log("Platform early init finished. Serial logging enabled.");

        //print out memory stats (now that we are able to)
        Memory::MemoryUsage memUsage = PageFrameAllocator::The()->GetMemoryUsage();
        string fstr = "Memory: 0x%llx bytes total, 0x%llx free, 0x%llx reserved, 0x%llx in-use.";
        Log(sl::FormatToString(0, &fstr, memUsage.total, memUsage.free, memUsage.reserved, memUsage.used).Data());

        //gather any cpu specific details
        CPU::Init();

        //print other fun facts
        fstr = "Kernel loaded at 0x%llx, size 0x%llx bytes.";
        Log(sl::FormatToString(0, &fstr, bootInfo->kernelStartAddr, bootInfo->kernelSize).Data());

        Log("GDT loaded at: 0x", false);
        Log(sl::UIntToString(defaultGdtDescriptor.offset, BASE_HEX).Data());
    }

    IDTR idtr;
    void InitInterrupts(BootInfo* bootInfo)
    {
        Log("Initializing interrupts");

        idtr.limit = 0x0FFF;
        idtr.offset = (uint64_t)PageFrameAllocator::The()->RequestPage();
        PageTableManager::The()->MapMemory((void*)idtr.offset, (void*)idtr.offset, MemoryMapFlags::WriteAllow | MemoryMapFlags::ExecuteAllow);

        //init cpu specific interrupts
        Log("Initializing platform interrupts");
        idtr.SetEntry((void*)InterruptHandlers::DoubleFault, INTERRUPT_VECTOR_DOUBLE_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::GeneralProtectionFault, INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::PageFault, INTERRUPT_VECTOR_PAGE_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::FPUError, INTERRUPT_VECTOR_FPU_ERROR, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::SIMDError, INTERRUPT_VECTOR_SIMD_ERROR, IDT_ATTRIBS_InterruptGate, 0x08);

        Log("Initializing software interrupts");
        //redirect entry for irq2 (ps2 keyboard)
        auto keyboardRedirect = IOAPIC::CreateRedirectEntry(INTERRUPT_VECTOR_PS2KEYBOARD, Drivers::APIC::Local()->GetID(), IOAPIC_PIN_POLARITY_ACTIVE_HIGH, IOAPIC_TRIGGER_MODE_EDGE, true);
        Drivers::IOAPIC::ioApics.PeekFront()->WriteRedirectEntry(1, keyboardRedirect);

        //ps2 keyboard
        idtr.SetEntry((void*)InterruptHandlers::PS2KeyboardHandler, INTERRUPT_VECTOR_PS2KEYBOARD, IDT_ATTRIBS_InterruptGate, 0x08);
        //scheduler timer callback
        idtr.SetEntry((void*)SchedulerTimerInterruptHandler, INTERRUPT_VECTOR_TIMER, IDT_ATTRIBS_InterruptGate, 0x08);

        //setting up irq0 (pin2) to send us ticks
        idtr.SetEntry((void*)InterruptHandlers::DefaultTimerHandler, INTERRUPT_VECTOR_TIMER_CALIBRATE, IDT_ATTRIBS_InterruptGate, 0x08);
        auto pitRedirect = IOAPIC::CreateRedirectEntry(INTERRUPT_VECTOR_TIMER_CALIBRATE, Drivers::APIC::Local()->GetID(), IOAPIC_PIN_POLARITY_ACTIVE_HIGH, IOAPIC_TRIGGER_MODE_EDGE, true);
        Drivers::IOAPIC::ioApics.PeekFront()->WriteRedirectEntry(2, pitRedirect); //TODO: magic numbers here!

        //load idt and enable interrupts
        CPU::LoadTable(CpuTable::X86_64_IDT, &idtr);
        CPU::EnableInterrupts();

        Log("IDT loaded at: 0x", false);
        Log(sl::UIntToString(idtr.offset, BASE_HEX).Data());
    }

    void InitDrivers(BootInfo* bootInfo)
    {
        Log("Initializing drivers.");

        Drivers::PIT::Init();
        Drivers::ACPI::The()->Init(reinterpret_cast<void*>(bootInfo->rsdp));
        Drivers::HPET::The()->Init();
        Drivers::APIC::Local()->Init();
        Drivers::IOAPIC::InitAll();
        Drivers::X86Extensions::Local()->Init();

        KRenderer::The()->Init(bootInfo);
    }

    void ExitKernelInit()
    {
        using namespace Kernel::Multiprocessing;
        Log("Exiting kernel init, preparing to run scheduled tasks.");

        Drivers::APIC::Local()->CalibrateTimer();

        //prepare to move to scheduled threads
        Scheduler::The()->Init();
        
        //setup kshell thread
        KernelThread* kShellThread = KernelThread::Create(Shell::KShell::ThreadMain, nullptr, 1);
        kShellThread->Start();

        //install scheduler handler, and start timer (TODO: move to SystemClock impl) and then yield immediately.
        Drivers::APIC::Local()->StartTimer(INTERRUPT_VECTOR_TIMER);
        Scheduler::The()->Yield();
    }
}

extern "C" __attribute__((noreturn)) void KernelMain(BootInfo* bootInfo)
{
    using namespace Kernel;

    InitMemory(bootInfo);
    InitPlatformEarly(bootInfo);

    LoadInitDisk();
    InitDrivers(bootInfo);
    InitInterrupts(bootInfo);

    ExitKernelInit();

    //someone we ended up back here, unwise to do anything else as we have no idea what state anything is in.
    LogError("[!] Kernel has returned to main function. [!]");
    while (1);
    __builtin_unreachable();
}