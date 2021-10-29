#include <drivers/APIC.h>
#include <drivers/ACPI.h>
#include <drivers/8259PIC.h>
#include <drivers/HPET.h>
#include <drivers/SystemClock.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Formatting.h>
#include <drivers/CPU.h>
#include <Maths.h>

namespace Kernel::Drivers
{
    sl::LinkedList<IOAPIC*> IOAPIC::ioApics;
    sl::List<IOApicSourceOverride> IOAPIC::ioApicSourceOverrides;

    uint32_t IOAPIC::ReadRegister(uint64_t offset)
    {
        /*  NOTE:
            The way this works is we have a select and read/write byte for interacting with the ioapic.
            The base address where we select what register to access, base + 0x10 is where we do i/o with the selected register.
            The same is true for reading the register
        */
        *(uint32_t*)virtualAddr = offset;
        return (*(uint32_t*)(virtualAddr + 0x10));
    }

    void IOAPIC::WriteRegister(uint64_t offset, uint32_t value)
    {
        *(uint32_t*)virtualAddr = offset;
        *(uint32_t*)(virtualAddr + 0x10) = value;
    }
    
    void IOAPIC::InitAll()
    {
        MADTHeader* madt = reinterpret_cast<MADTHeader*>(ACPI::The()->FindHeader("APIC"));
        if (madt == nullptr)
        {
            LogError("Unable to locate MADT, cannot initialize IOAPICs.");
            return;
        }

        //more than any known ioapic is known to have, so this should be fine. It can expand dynamically if needed.
        ioApicSourceOverrides.Reserve(0x20);

        uint64_t madtLength = (uint64_t)madt + madt->header.length;
        MADTEntry* entry = reinterpret_cast<MADTEntry*>((uint64_t)madt + 0x2C);
        while ((uint64_t)entry < madtLength)
        {
            switch (entry->type)
            {
            case MADTEntryType::IOAPIC:
            {
                uint8_t apicId = reinterpret_cast<uint8_t>(*(uint8_t*)((uint64_t)entry + 0x2));
                uint32_t physicalAddr = reinterpret_cast<uint32_t>(*(uint32_t*)((uint64_t)entry + 0x4));
                uint32_t gsiBase = reinterpret_cast<uint32_t>(*(uint32_t*)((uint64_t)entry + 0x8));

                IOAPIC* ioApic = new IOAPIC();
                ioApic->Init(apicId, physicalAddr, gsiBase);
                ioApics.PushBack(ioApic);
                break;
            }

            case MADTEntryType::IOAPIC_SourceOverride:
            {
                IOApicSourceOverride sourceOverride;
                sourceOverride.busSource = reinterpret_cast<uint8_t>(*(uint8_t*)((uint64_t)entry + 0x2));
                sourceOverride.irqSource = reinterpret_cast<uint8_t>(*(uint8_t*)((uint64_t)entry + 0x3));
                sourceOverride.globalSystemInterrupt = reinterpret_cast<uint32_t>(*(uint32_t*)((uint64_t)entry + 0x4));
                sourceOverride.flags.byte = reinterpret_cast<uint16_t>(*(uint16_t*)((uint64_t)entry + 0x8)); //narrowing, but the upper 8 bits arent used anyway.
                ioApicSourceOverrides.InsertAt(sourceOverride.busSource, sourceOverride);

                string format = "IOAPIC Source override: bus=0x%x, irq=0x%x, gsi=0x%x, flags=0x%x";
                Log(sl::FormatToString(0, &format, sourceOverride.busSource, sourceOverride.irqSource, sourceOverride.globalSystemInterrupt, sourceOverride.flags.byte).Data());
                break;
            }

            case MADTEntryType::IOAPIC_NonMaskableSourceOverride:
                Log("Got IOAPIC NM source override.");
                break;
            }

            entry = reinterpret_cast<MADTEntry*>((uint64_t)entry + entry->length);
        }
    }

    IOApicRedirectEntry IOAPIC::CreateRedirectEntry(uint8_t gsiVector, uint8_t physDestination, uint8_t pinPolarity, uint8_t triggerMode, bool enabled)
    {
        IOApicRedirectEntry entry;
        entry.vector = gsiVector;
        entry.deliveryMode = IOAPIC_DELIVERY_MODE_FIXED;
        entry.destinationMode = IOAPIC_DESTINATION_PHYSICAL;
        entry.pinPolarity = pinPolarity;
        entry.triggerMode = triggerMode;
        entry.mask = enabled ? IOAPIC_MASK_ENABLE : IOAPIC_MASK_DISABLE;
        entry.destinationMode = physDestination;
        return entry;
    }
    
    void IOAPIC::Init(uint8_t apicId, uint32_t physAddr, uint32_t gsiBase)
    {
        id = apicId;
        physicalAddr = virtualAddr = physAddr;
        globalInterruptBase = gsiBase;

        //ensure page container registers is identity mapped.
        PageFrameAllocator::The()->LockPage((void*)physicalAddr);
        PageTableManager::The()->MapMemory((void*)virtualAddr, (void*)physicalAddr, MemoryMapFlags::WriteAllow | MemoryMapFlags::EternalClaim);

        inputCount = (ReadRegister(IOAPIC_REGISTER_VERSION_MAXREDIRECTS) >> 16) + 1;

        string format = "IOAPIC initialized at: 0x%X, id=0x%X, inputs=0x%X, gsi_base=%x";
        Log(sl::FormatToString(0, &format, physAddr, apicId, inputCount, gsiBase).Data());
    }

    void IOAPIC::WriteRedirectEntry(uint8_t entryNum, const IOApicRedirectEntry& entry)
    {
        WriteRegister(IOAPIC_REGISTER_REDIRECT_START + (entryNum * 2), entry.packedLowerHalf);
        WriteRegister(IOAPIC_REGISTER_REDIRECT_START + (entryNum * 2) + 1, entry.packedUpperHalf);
    }

    IOApicRedirectEntry IOAPIC::ReadRedirectEntry(uint8_t entryNum)
    {
        IOApicRedirectEntry entry;
        entry.packedLowerHalf = ReadRegister(IOAPIC_REGISTER_REDIRECT_START + (entryNum * 2));
        entry.packedUpperHalf = ReadRegister(IOAPIC_REGISTER_REDIRECT_START + (entryNum * 2) + 1);        
        return entry;
    }

    uint32_t APIC::CreateTimerLVT(uint8_t vector, uint8_t timerMode, bool enabled)
    {
        return vector | (timerMode << 17) | ((enabled ? 0 : 1) << 16);
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
        if (!ACPI::ChecksumValid((SDTHeader*)madt))
        {
            LogError("MADT Header is corrupted, checksum mismatch. Aborting APIC init.");
            return;
        }

        if ((madt->flags & MADT_FLAGS_DUAL8259_INSTALLED) != 0)
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
        Log(sl::UIntToString(madt->localAddress, BASE_HEX).Data());
        localApicAddr = reinterpret_cast<uint32_t*>(madt->localAddress);

        //ensure page that contains LAPIC registers is locked and identity mapped
        PageFrameAllocator::The()->LockPage(localApicAddr);
        PageTableManager::The()->MapMemory(localApicAddr, localApicAddr, MemoryMapFlags::WriteAllow | MemoryMapFlags::EternalClaim);

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
            Log(sl::UIntToString(entry->length, BASE_DECIMAL).Data());

            entry = (MADTEntry*)((uint64_t)entry + entry->length);
        }

        Log("Total processors: ", false);
        Log(sl::UIntToString(totalProcessors, BASE_DECIMAL).Data());
    }

    uint32_t APIC::ReadRegister(LocalApicRegisters reg)
    {
        return *(localApicAddr + ((uint64_t)reg * 4));
    }

    void APIC::WriteRegister(LocalApicRegisters reg, uint32_t value)
    {
        localApicAddr[((uint64_t)reg) * 4] = value;
    }

    void APIC::CalibrateTimer()
    {
        //setup APIC timer without starting it.
        WriteRegister(LocalApicRegisters::TimerDivideConfig, 0x3);

        //set initial count to reset apic timer, and start HPET timer.
        WriteRegister(LocalApicRegisters::TimerInitialCount, 0xFFFF'FFFF);

        volatile uint64_t* ticksPtr = &SystemClock::The()->clockTicks;
        const uint64_t endTicks = *ticksPtr + APIC_TIMER_DESIRED_MS;
        while (*ticksPtr < endTicks);

        //get number of apic clock ticks, 
        timerInterval = 0xFFFF'FFFF - ReadRegister(LocalApicRegisters::TimerCurrentCount);
        Log("APIC timer calibrated for tick count: ", false);
        Log(sl::UIntToString(timerInterval, 10).Data());

        //stop apic timer, HPET was 1 shot so no need to disable it.
        WriteRegister(LocalApicRegisters::TimerInitialCount, 0x0);
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
        WriteRegister(LocalApicRegisters::EOI, 0);
    }

    void APIC::StartTimer(uint8_t interruptVectorNum)
    {
        WriteRegister(LocalApicRegisters::TimerDivideConfig, 0x3);
        WriteRegister(LocalApicRegisters::LvtTimer, CreateTimerLVT(interruptVectorNum, LAPIC_TIMER_MODE_PERIODIC, true));
        WriteRegister(LocalApicRegisters::TimerInitialCount, timerInterval);
    }

    uint8_t APIC::GetID()
    {
        return (uint8_t)ReadRegister(LocalApicRegisters::ID);
    }

    void APIC::SendInitIPI(uint64_t apicId)
    {
        ICREntry entry;
        entry.level = 1; //must be high
        entry.destination = apicId;
        entry.deliveryMode = 0b101; //TODO: magic numbers and APIC cleanup

        SendIPI(entry);
    }

    void APIC::SendStartupIPI(uint64_t apicId, uint8_t vector)
    {
        ICREntry entry;
        entry.level = 1;
        entry.destination = apicId;
        entry.deliveryMode = 0b110;
        entry.remoteVector = vector;

        SendIPI(entry);
    }

    void APIC::SendIPI(ICREntry details)
    {
        if (details.deliveryMode != 0b101) //only INIT can issue a de-assert, all others MUST be assert
            details.level = 1;

        WriteRegister(LocalApicRegisters::InterruptCommand1, details.squish >> 32);
        WriteRegister(LocalApicRegisters::InterruptCommand0, details.squish);
    }
}