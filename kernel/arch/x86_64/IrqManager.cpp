#include <IrqManager.h>
#include <arch/x86_64/IDT.h>
#include <drivers/CPU.h>
#include <drivers/APIC.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Memory.h>
#include <Platform.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Formatting.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64);

extern "C" 
{
    extern uint8_t IrqDefaultEntry;
    extern uint8_t IrqDefaultEntryEnd;
    
    uint64_t irqSaveStack;
    
    void IrqManager_Dispatch(MinosInterruptFrame* frame)
    {
        Kernel::IrqManager::The()->SharedIrqDispatch(frame);
    }
}

namespace Kernel
{
    void IrqManager::SharedIrqDispatch(MinosInterruptFrame* frame)
    {
        SendEOI();
    }
    
    IrqManager irqManagerInstance;
    IrqManager* IrqManager::The()
    {
        return &irqManagerInstance;
    }

    IDTR localIdtr; //has to be defined here so header can be platform independant
    void IrqManager::Init()
    {
        Log("Initializing IrqManager, blanking IDT.");
        
        //setup blank idt
        localIdtr.limit = 0x0FFF; //max on x86 arch
        localIdtr.offset = reinterpret_cast<uint64_t>(PageFrameAllocator::The()->RequestPage());
        PageTableManager::The()->MapMemory((void*)localIdtr.offset, (void*)localIdtr.offset, MemoryMapFlags::WriteAllow | MemoryMapFlags::EternalClaim);
        sl::memset((void*)localIdtr.offset, 0, PAGE_SIZE);

        irqSaveStack = 0;

        //TODO: create storage for dummy idt entries, and patch code in-place.
    }

    void IrqManager::Load()
    {
        //load IDT, and enable interrupts
        Drivers::CPU::LoadTable(Drivers::CpuTable::X86_64_IDT, &localIdtr);
        Drivers::CPU::EnableInterrupts();

        Log("Loaded new IDT: 0x", false);
        Log(sl::UIntToString(localIdtr.offset, 16).Data());
    }

    bool IrqManager::IsArchitectural(IrqVector vector)
    {
        return vector < 0x20;
    }

    bool IrqManager::IsAvailable(IrqVector vector)
    {
        if (vector < 0x20 || vector > 0xFF)
            return false;
        
        IDTEntry* entry = reinterpret_cast<IDTEntry*>(localIdtr.offset + vector * sizeof(IDTEntry));
        return entry->attributes.present == 0;
    }

    IrqVector IrqManager::GetFree()
    {
        for (size_t index = 0x20; index < 0x100; index++)
        {
            IDTEntry* entry = reinterpret_cast<IDTEntry*>(localIdtr.offset + index * sizeof(IDTEntry));
            if (!entry->attributes.present)
                return index;
        }

        return 0;
    }

    IrqVector IrqManager::GetFreeFrom(sl::List<IrqVector> vectors)
    {
        for (size_t index = 0; index < vectors.Size(); index++)
        {
            IDTEntry* entry = reinterpret_cast<IDTEntry*>(localIdtr.offset + vectors[index] * sizeof(IDTEntry));
            if (!entry->attributes.present)
                return index;
        }

        return 0;
    }

    bool IrqManager::AssignVector(IrqVector vector, void* handler, bool exclusive)
    { //TODO: implement non-exclusive irqs -> need a hashmap first
        if (vector > 0xFF)
            return false;

        IDTEntry* entry = reinterpret_cast<IDTEntry*>(localIdtr.offset + vector * sizeof(IDTEntry));
        if (entry->attributes.present)
        {
            Log("Attempted to overwrite IDT entry, ignoring request. Vector=0x", false);
            Log(sl::UIntToString(vector, 16).Data());
            return false;
        }

        //zero entry so we're coming from a known state
        sl::memset(entry, 0, sizeof(IDTEntry));

        entry->SetOffset((uint64_t)handler);
        entry->selector = 0x8; //TODO: get code selector automtically some how
        entry->attributes.present = 1;
        entry->attributes.gateType = IDT_GATE_TYPE_INTERRUPT;

        string fstr = "Updated entry for irq entry 0x%lx: sel=0x%x, gate=0x%x, entry=0x%lx, exclusive=%b";
        Log(sl::FormatToString(0, &fstr, vector, entry->selector, IDT_GATE_TYPE_INTERRUPT, (uint64_t)handler).Data(), exclusive);

        return true;
    }

    void IrqManager::ClearVector(IrqVector vector)
    {
        IDTEntry* entry = reinterpret_cast<IDTEntry*>(localIdtr.offset + vector * sizeof(IDTEntry));
        sl::memset(entry, 0, sizeof(IDTEntry));

        Log("Forcibly cleared IDT entry 0x", false);
        Log(sl::UIntToString(vector, 16).Data());
    }

    void IrqManager::SendEOI()
    {
        //this is where we would select whether to communicate with PIC or APIC
        Drivers::APIC::Local()->SendEOI();
    }

    void IrqManager::PrintVectorTable()
    {
        Log("No. (not implemented yet)");
    }
}
