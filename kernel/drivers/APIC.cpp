#include <drivers/APIC.h>
#include <drivers/ACPI.h>
#include <drivers/8259PIC.h>
#include <PageTableManager.h>
#include <KLog.h>
#include <StringUtil.h>
#include <CPU.h>

namespace Kernel::Drivers
{
    Syslib::LinkedList<IOAPIC*> ioApics;
    
    void IOAPIC::InitAll()
    {
        MADTHeader* madt = reinterpret_cast<MADTHeader*>(ACPI::The()->FindHeader("APIC"));
        if (madt == nullptr)
        {
            LogError("Unable to locate MADT, cannot initialize IOAPICs.");
            return;
        }

        uint64_t madtLength = (uint64_t)madt + madt->header.length;
        MADTEntry* entry = reinterpret_cast<MADTEntry*>((uint64_t)madt + 0x2C);
        while ((uint64_t)entry < madtLength)
        {
            switch (entry->type)
            {
                case MADTEntryType::IOAPIC:
                    uint8_t apicId = reinterpret_cast<uint8_t>(*(uint8_t*)((uint64_t)entry + 0x2));
                    uint32_t* apicAddress = reinterpret_cast<uint32_t*>((uint64_t)entry + 0x4);
                    uint32_t* gsiBase = reinterpret_cast<uint32_t*>((uint64_t)entry + 0x8);

                    //TODO: dont identity map this, virtual addr can be obtained through RequestPage(), and we just write to that.
                    PageTableManager::The()->MapMemory(apicAddress, apicAddress);

                    IOAPIC* ioApic = new IOAPIC();
                    ioApic->Init(apicId, apicAddress, gsiBase);
                    ioApics.PushBack(ioApic);
                    break;
            }

            entry = reinterpret_cast<MADTEntry*>((uint64_t)entry + entry->length);
        }
    }
    
    void IOAPIC::Init(uint8_t apicId, uint32_t* address, uint32_t* gsiBase)
    {
        id = apicId;
        baseAddress = address;
        globalInterruptBase = gsiBase;

        Log("IOAPIC initialized at: 0x", false);
        Log(ToStrHex((uint64_t)address), false);
        Log(", id=0x", false);
        Log(ToStrHex(apicId));
    }
    
    APIC localApic;
    APIC* APIC::Local()
    {
        return &localApic;
    }

    void APIC::Init()
    {
        if (!CPU::FeatureSupported(CpuFeatureFlag::APIC))
        {
            LogError("APIC unavailable, aborting apic init.");
            return;
        }

        MADTHeader* madt = reinterpret_cast<MADTHeader*>(ACPI::The()->FindHeader("APIC"));
        if (madt == nullptr)
        {
            LogError("CPU reports APIC available, but not table was found in ACPI. Aborting init.");
            return;
        }

        if ((madt->flags & MADT_FLAGS_DUAL8259_INSTALLED) != 0 || FORCE_DISABLE_LEGACY_PIC)
        {
            //remap PICs so any spurious interrupts dont cause exceptions, then disable them outright.
            PIC::Remap();
            CPU::PortWrite8(PORT_PIC1_DATA, 0b1111'1111);
            CPU::PortIOWait();
            CPU::PortWrite8(PORT_PIC2_DATA, 0b1111'1111);
            PIC::Disable();

            Log("APIC has disabled legacy 8259 PICs.");
        }

        Log("Local APIC found at: 0x", false);
        Log(ToStrHex(madt->localAddress));
        localApicAddr = reinterpret_cast<uint32_t*>(madt->localAddress);

        //ensure page that contains LAPIC registers is locked and identity mapped
        PageTableManager::The()->MapMemory(localApicAddr, localApicAddr);

        //This 'refreshes' the LAPIC, enabling the hardware if it was disabled at boot for some reason.
        SetLocalBase(GetLocalBase());

        //setting bit 8 spurious interrupt vector fully enables the APIC, now it's active and will register interrupts
        WriteRegister(LocalApicRegisters::SpuriousInterruptVector, 0x100);
    }

    void APIC::PrintTablesInfo()
    {
        MADTHeader* madt = reinterpret_cast<MADTHeader*>(ACPI::The()->FindHeader("APIC"));
        if (madt == nullptr)
        {
            LogError("Unable to find MADT, cannot print APIC info.");
            return;
        }

        uint64_t totalProcessors = 0;
        uint64_t madtLength = (uint64_t)madt + madt->header.length;
        MADTEntry* entry = reinterpret_cast<MADTEntry*>((uint64_t)madt + 0x2C); //0x2C is the end of the MADT header, there is some reserved space after the known fields.
        while ((uint64_t)entry < madtLength)
        {
            switch (entry->type) 
            {
                case MADTEntryType::LocalAPIC:
                    Log("Entry Type: Local APIC");
                    totalProcessors++;
                    break;
                case MADTEntryType::IOAPIC:
                    Log("Entry Type: IO APIC");
                    break;
                case MADTEntryType::IOAPIC_SourceOverride:
                    Log("Entry Type: IO APIC, source override");
                    break;
                case MADTEntryType::IOAPIC_NonMaskableSourceOverride:
                    Log("Entry Type: IO APIC, NM source override");
                    break;
                case MADTEntryType::LocalAPIC_NonMaskableInterrupts:
                    Log("Entry Type: LAPIC, NMI");
                    break;
                case MADTEntryType::LocalAPIC_AddressOverride:
                    Log("Entry Type: LAPIC, addr override");
                    break;
                case MADTEntryType::Localx2APIC:
                    Log("Entry Type: Local x2APIC");
                    break;
            }
            Log("Entry Length: 0x", false);
            Log(ToStr(entry->length));

            entry = (MADTEntry*)((uint64_t)entry + entry->length);
        }

        Log("Total processors: ", false);
        Log(ToStr(totalProcessors));
    }

    uint32_t APIC::ReadRegister(LocalApicRegisters reg)
    {
        return *(localApicAddr + ((uint64_t)reg * 4));
    }

    void APIC::WriteRegister(LocalApicRegisters reg, uint32_t value)
    {
        localApicAddr[((uint64_t)reg) * 4] = value;
    }

    uint64_t APIC::GetLocalBase()
    {
        return CPU::ReadMSR(APIC_BASE_MSR) & 0xffff'f000; //filter out control bits
    }

    void APIC::SetLocalBase(uint64_t addr)
    {
        CPU::WriteMSR(APIC_BASE_MSR, addr & 0xffff'f000);
        localApicAddr = (uint32_t*)addr;
    }

    void APIC::SendEOI()
    {
        WriteRegister(LocalApicRegisters::EOI, 1);
    }

    void APIC::StartTimer(uint8_t interruptVectorNum)
    {
        WriteRegister(LocalApicRegisters::TimerDivideConfig, 0x3);
        //TODO: implement a nice way to r/w LVTs
        WriteRegister(LocalApicRegisters::LvtTimer, 0x20'000 | interruptVectorNum); //0x20000 = period mode, 0x20 = irq0
        WriteRegister(LocalApicRegisters::TimerInitialCount, 0x2000); //TODO: use external clock to set this to a known value.
    }
}