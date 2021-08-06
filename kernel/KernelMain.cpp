#include <BootInfo.h>
#include <KRenderer.h>
#include <PageFrameAllocator.h>
#include <PageTableManager.h>
#include <memory/Utilities.h>
#include <CPU.h>
#include <IDT.h>
#include <Interrupts.h>
#include <Serial.h>
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
    void PrepareMemory(BootInfo* bootInfo)
    {
        PageFrameAllocator::The()->Init(bootInfo);

        //lock the pages used by the kernel
        uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
        uint64_t kernelPages = kernelSize / PAGE_SIZE + 1;
        PageFrameAllocator::The()->LockPages(&_KernelStart, kernelPages);

        PageTable* plm4 = (PageTable*)PageFrameAllocator::The()->RequestPage();
        memset(plm4, 0, PAGE_SIZE);

        PageTableManager* kernelPageTableManager = PageTableManager::The();
        kernelPageTableManager->Init(plm4); 

        //identity map all pages (for now)
        for (uint64_t i = 0; i < PageFrameAllocator::The()->GetTotalMemory(); i += PAGE_SIZE)
        {
            kernelPageTableManager->MapMemory((void*)i, (void*)i);
        }

        //ensure GOP framebuffer is still mapped where we expect it to be (TODO: this may be useless if we switch to another graphics driver)
        uint64_t fbBase = (uint64_t)bootInfo->gop.baseAddress;
        uint64_t fbSize = (uint64_t)bootInfo->gop.bufferSize + PAGE_SIZE;
        PageFrameAllocator::The()->LockPages((void*)fbBase, fbSize / PAGE_SIZE + 1);

        for (uint64_t i = fbBase; i < fbBase + fbSize; i += PAGE_SIZE)
        {
            kernelPageTableManager->MapMemory((void*)i, (void*)i);
        }
        kernelPageTableManager->MakeCurrentMap();

        //initialize kernel heap at arbitrarily large address to stay out of the way of everything else
        //"the beauty of virtual memory"
        KHeap::The()->Init((void*)0x100000000, 0x1000);
    }

    IDTR idtr;
    void PrepareInterrupts(BootInfo* bootInfo)
    {
        idtr.limit = 0x0fff;
        idtr.offset = (uint64_t)PageFrameAllocator::The()->RequestPage();

        //CPU interrupts
        idtr.SetEntry((void*)InterruptHandlers::DoubleFault, INTERRUPT_VECTOR_DOUBLE_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::GeneralProtectionFault, INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr.SetEntry((void*)InterruptHandlers::PageFault, INTERRUPT_VECTOR_PAGE_FAULT, IDT_ATTRIBS_InterruptGate, 0x8);

        //write redirect entry for irq2 (ps2 keyboard)
        Drivers::IOApicRedirectEntry ps2RedirectEntry;
        ps2RedirectEntry.vector = INTERRUPT_VECTOR_PS2KEYBOARD;
        ps2RedirectEntry.deliveryMode = IOAPIC_DELIVERY_MODE_FIXED;
        ps2RedirectEntry.destinationMode = IOAPIC_DESTINATION_PHYSICAL;
        ps2RedirectEntry.pinPolarity = IOAPIC_PIN_POLARITY_ACTIVE_HIGH;
        ps2RedirectEntry.triggerMode = IOAPIC_TRIGGER_MODE_EDGE;
        ps2RedirectEntry.mask = IOAPIC_MASK_ENABLE;
        ps2RedirectEntry.destination = Drivers::APIC::Local()->GetID();
        Drivers::IOAPIC::ioApics.PeekFront()->WriteRedirectEntry(1, ps2RedirectEntry);
        //PIC interrupts
        idtr.SetEntry((void*)InterruptHandlers::PS2KeyboardHandler, INTERRUPT_VECTOR_PS2KEYBOARD, IDT_ATTRIBS_InterruptGate, 0x08);
        //idtr.SetEntry((void*)InterruptHandlers::DefaultTimerHandler, INTERRUPT_VECTOR_TIMER, IDT_ATTRIBS_InterruptGate, 0x8); 
        idtr.SetEntry((void*)SchedulerTimerInterruptHandler, INTERRUPT_VECTOR_TIMER, IDT_ATTRIBS_InterruptGate, 0x08);

        CPU::LoadIDT(&idtr);
        CPU::EnableInterrupts();

        Log("IDT with offset: 0x", false);
        Log(ToStrHex(idtr.offset));
    }

    void PrepareDrivers(BootInfo* bootInfo)
    {   
        Drivers::ACPI::The()->Init(bootInfo->rsdp);
        Drivers::APIC::Local()->Init();
        Drivers::IOAPIC::InitAll();
        
        KRenderer::The()->Init(bootInfo);
        SetRenderedLogging(true);
        Log("KRenderer initialized.");
    }
}

extern "C" __attribute__((noreturn)) void KernelMain(BootInfo* bootInfo)
{
    using namespace Kernel;

    GDTDescriptor gdtDescriptor;
    gdtDescriptor.size = sizeof(GDT) - 1;
    gdtDescriptor.offset = (uint64_t)&defaultGdt;
    CPU::LoadGDT(&gdtDescriptor);

    CPU::Init();
    PrepareMemory(bootInfo);
    
    //call any non trivial global constructors now that we have memory setup.
    _init();

    SerialPort::COM1()->Init(PORT_COM1_ADDRESS); //COM1 gets initialized here so we have logging output.
    SetSerialLogging(true);
    Log("COM1 serial initialized. Welcome, user.");

    PrepareDrivers(bootInfo);
    PrepareInterrupts(bootInfo);

    //setup logging for the rest of the boot process (we should do this sooner rather than later)
    Log("Kernel booted.");

    //init scheduler, and spawn shell thread
    Multiprocessing::Scheduler::The()->Init();
    Multiprocessing::KernelThread* shellThread = Multiprocessing::KernelThread::Create(Shell::KShell::ThreadMain, nullptr, 1);
    shellThread->Start();

    //start timer (scheduler handler should already be installed), and yield to scheduler
    Drivers::APIC::Local()->StartTimer(INTERRUPT_VECTOR_TIMER);
    Multiprocessing::Scheduler::The()->Yield();

    LogError("Scheduler return!");
    while (1);

    __builtin_unreachable();
}