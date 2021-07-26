#include "BootInfo.h"
#include "KRenderer.h"
#include "PageFrameAllocator.h"
#include "PageTableManager.h"
#include "Memory.h"
#include "CPU.h"
#include "IDT.h"
#include "Interrupts.h"
#include "Serial.h"
#include "KLog.h"
#include "StringUtil.h"
#include "Panic.h"
#include "PIC.h"
#include "memory/KHeap.h"

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

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

        IDTDescriptorEntry* interruptDoubleFault = (IDTDescriptorEntry*)(idtr.offset + 0x8 * sizeof(IDTDescriptorEntry));
        interruptDoubleFault->SetOffset((uint64_t)InterruptHandlers::DoubleFault);
        interruptDoubleFault->attributes = IDT_ATTRIBS_InterruptGate;
        interruptDoubleFault->selector = 0x08;

        IDTDescriptorEntry* interruptGeneralProtection = (IDTDescriptorEntry*)(idtr.offset + 0xD * sizeof(IDTDescriptorEntry));
        interruptGeneralProtection->SetOffset((uint64_t)InterruptHandlers::GeneralProtectionFault);
        interruptGeneralProtection->attributes = IDT_ATTRIBS_InterruptGate;
        interruptGeneralProtection->selector = 0x08;
        
        IDTDescriptorEntry* interruptPageFault = (IDTDescriptorEntry*)(idtr.offset + 0xE * sizeof(IDTDescriptorEntry));
        interruptPageFault->SetOffset((uint64_t)InterruptHandlers::PageFault);
        interruptPageFault->attributes = IDT_ATTRIBS_InterruptGate;
        interruptPageFault->selector = 0x08; //kernel code

        //getting ready for PIC interrupts
        PIC::Remap();
        CPU::PortWrite8(PORT_PIC1_DATA, 0b11111101); //2nd bit is keyboard interrupt, enable it
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PIC2_DATA, 0b11111111);

        IDTDescriptorEntry* interruptPs2Keyboard = (IDTDescriptorEntry*)(idtr.offset + (PIC1_IDT_OFFSET + 0x1) * sizeof(IDTDescriptorEntry));
        interruptPs2Keyboard->SetOffset((uint64_t)InterruptHandlers::PS2KeyboardHandler);
        interruptPs2Keyboard->attributes = IDT_ATTRIBS_InterruptGate;
        interruptPs2Keyboard->selector = 0x08;

        CPU::LoadIDT(&idtr);
        CPU::EnableInterrupts();

        Log("Interrupt Descriptor Table with offset: 0x", false);
        Log(ToStrHex((uint64_t)idtr.offset));
    }

    void PrepareDrivers(BootInfo* bootInfo)
    {   
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

    PrepareMemory(bootInfo);

    SerialPort::COM1()->Init(PORT_COM1_ADDRESS); //COM1 gets initialized here so we have logging output.
    SetSerialLogging(true);
    Log("COM1 serial initialized. Welcome, user.");

    PrepareInterrupts(bootInfo);
    PrepareDrivers(bootInfo);

    //setup logging for the rest of the boot process (we should do this sooner rather than later)
    Log("Kernel booted.");

    while (1);

    __builtin_unreachable();
}