#include <drivers/APIC.h>
#include <drivers/ACPI.h>
#include <drivers/8259PIC.h>
#include <KLog.h>
#include <StringUtil.h>
#include <CPU.h>

namespace Kernel::Drivers
{
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
        localApicAddr = madt->localAddress;

        PrintTablesInfo();
    }

    void APIC::PrintTablesInfo()
    {
        MADTHeader* madt = reinterpret_cast<MADTHeader*>(ACPI::The()->FindHeader("APIC"));
        if (madt == nullptr)
        {
            LogError("Unable to find MADT, cannot print APIC info.");
            return;
        }

        uint64_t madtLength = (uint64_t)madt + madt->header.length;
        MADTEntry* entry = reinterpret_cast<MADTEntry*>((uint64_t)madt + 0x2C);
        while ((uint64_t)entry < madtLength)
        {
            Log("Entry type: ", false);
            Log(ToStrHex(entry->type));
            Log("Entry length: ", false);
            Log(ToStrHex(entry->length));

            entry = (MADTEntry*)((uint64_t)entry + entry->length);
        }
    }

    uint64_t APIC::GetLocalBase()
    {
        return 0;
    }
}