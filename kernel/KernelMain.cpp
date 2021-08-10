#include <BootInfo.h>
#include <KRenderer.h>
#include <PageFrameAllocator.h>
#include <PageTableManager.h>
#include <memory/Utilities.h>
#include <drivers/CPU.h>
#include <IDT.h>
#include <Interrupts.h>
#include <drivers/Serial.h>
#include <KLog.h>
#include <StringUtil.h>
#include <Panic.h>
#include <memory/KHeap.h>
#include <drivers/ACPI.h>
#include <drivers/APIC.h>
#include <multiprocessing/Scheduler.h>
#include <kshell/KShell.h>

extern "C"
{
extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;
extern void _init();
}

namespace Kernel
{
    using namespace Kernel::Drivers;

    extern void InitPlatformInterrupts(IDTR* idtr);

    void InitMemory(BootInfo* bootInfo)
    {
        //assemble memory map into something we can use.
        PageFrameAllocator::The()->Init(bootInfo);

        //get size of kernel in pages, so we can protect the space used by the binary
        uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
        uint64_t kernelPages = kernelSize / PAGE_SIZE + 1; //round up to nearest page
        PageFrameAllocator::The()->LockPages(&_KernelStart, kernelPages);

        //create page table, init it as kernel table
        PageTable* kernelPageTable = (PageTable*)PageFrameAllocator::The()->RequestPage();
        memset(kernelPageTable, 0, PAGE_SIZE); //zero page table = everything is absent

        PageTableManager::The()->Init(kernelPageTable);

        //identity map all pages for now. TODO: only map what we need
        for (uint64_t i = 0; i < PageFrameAllocator::The()->GetTotalMemory(); i += PAGE_SIZE)
            PageTableManager::The()->MapMemory((void*)i, (void*)i);

        //identity map GOP framebuffer and lock pages
        uint64_t fbBase = (uint64_t)bootInfo->gop.baseAddress;
        uint64_t fbSize = (uint64_t)bootInfo->gop.bufferSize + PAGE_SIZE;
        PageFrameAllocator::The()->LockPages((void*)fbBase, fbSize / PAGE_SIZE + 1);

        for (uint64_t i = fbBase; i < fbBase + fbSize; i += PAGE_SIZE)
            PageTableManager::The()->MapMemory((void*)i, (void*)i);

        PageTableManager::The()->MakeCurrentMap();

        //initialize kernel heap at an arbitrarily large address
        KHeap::The()->Init((void*)0x100'000'000, 0x1000);
    }

    void InitPlatformEarly(BootInfo* bootInfo)
    {
        //load out default gdt
        GDTDescriptor rootDescriptor;
        rootDescriptor.size = sizeof(GDT) - 1;
        rootDescriptor.offset = (uint64_t)&defaultGdt;
        CPU::LoadGDT(&rootDescriptor);

        //gather any cpu specific details
        CPU::Init();
        
        //call global constructors
        _init();

        //get serial logging up as early as we can
        InitLogging();
        SerialPort::COM1()->Init(PORT_COM1_ADDRESS);
        SetSerialLogging(true);
        Log("Platform early init finished. Serial logging enabled.");

        Log("GDT loaded at: 0x", false);
        Log(ToStrHex(rootDescriptor.offset));
    }

    IDTR idtr;
    void InitInterrupts(BootInfo* bootInfo)
    {
        Log("Initializing interrupts");

        idtr.limit = 0x0FFF;
        idtr.offset = (uint64_t)PageFrameAllocator::The()->RequestPage();

        //init any platform specific interrupts
        Log("Initializing platform specific interrupts");
        InitPlatformInterrupts(&idtr);

        //redirect entry for irq2 (ps2 keyboard)
        Drivers::IOApicRedirectEntry ps2RedirectEntry;
        ps2RedirectEntry.vector = INTERRUPT_VECTOR_PS2KEYBOARD;
        ps2RedirectEntry.deliveryMode = IOAPIC_DELIVERY_MODE_FIXED;
        ps2RedirectEntry.destinationMode = IOAPIC_DESTINATION_PHYSICAL;
        ps2RedirectEntry.pinPolarity = IOAPIC_PIN_POLARITY_ACTIVE_HIGH;
        ps2RedirectEntry.triggerMode = IOAPIC_TRIGGER_MODE_EDGE;
        ps2RedirectEntry.mask = IOAPIC_MASK_ENABLE;
        ps2RedirectEntry.destination = Drivers::APIC::Local()->GetID();
        Drivers::IOAPIC::ioApics.PeekFront()->WriteRedirectEntry(1, ps2RedirectEntry);

        //ps2 keyboard
        idtr.SetEntry((void*)InterruptHandlers::PS2KeyboardHandler, INTERRUPT_VECTOR_PS2KEYBOARD, IDT_ATTRIBS_InterruptGate, 0x08);
        //scheduler timer callback
        idtr.SetEntry((void*)SchedulerTimerInterruptHandler, INTERRUPT_VECTOR_TIMER, IDT_ATTRIBS_InterruptGate, 0x08);

        //load idt and enable interrupts
        CPU::LoadIDT(&idtr);
        CPU::EnableInterrupts();

        Log("IDT loaded at: 0x", false);
        Log(ToStrHex(idtr.offset));
    }

    void InitDrivers(BootInfo* bootInfo)
    {
        Log("Initializing drivers.");

        Drivers::ACPI::The()->Init(bootInfo->rsdp);
        Drivers::APIC::Local()->Init();
        Drivers::IOAPIC::InitAll();

        KRenderer::The()->Init(bootInfo);
    }

    void ExitKernelInit()
    {
        using namespace Kernel::Multiprocessing;
        Log("Exiting kernel init, preparing to run scheduled tasks.");

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

    InitDrivers(bootInfo);
    InitInterrupts(bootInfo);

    ExitKernelInit();

    //someone we ended up back here, unwise to do anything else as we have no idea what state anything is in.
    LogError("[!] Kernel has returned to main function. [!]");
    while (1);
    __builtin_unreachable();
}