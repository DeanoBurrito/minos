#include <drivers/HPET.h>
#include <drivers/ACPI.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <memory/KHeap.h>
#include <PlacementNew.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Formatting.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64);

namespace Kernel::Drivers
{
    HPETComparator::HPETComparator(const TimerConfigCapabilities caps, const uint8_t id) : index(id), capabilities(caps), currentConfig(caps)
    {}

    uint32_t HPETComparator::GetAllowedRoutingBitmap() const
    { return capabilities.allowedRoutesBitfield; }

    bool HPETComparator::IsEnabled() const
    { return capabilities.enable; }

    bool HPETComparator::HasPeriodicMode() const
    { return capabilities.periodicSupported; }

    bool HPETComparator::EmitsLevelTriggers() const
    { return capabilities.generatesLevelTriggers; }

    bool HPETComparator::FullWidthTimer() const
    { return !capabilities.forceSmallTimer; }

    bool HPETComparator::SupportsFSBRouting() const
    { return capabilities.fsbRoutingSupported; }

    bool HPETComparator::EnableFSBRouting(const bool yes)
    {
        if (!capabilities.fsbRoutingSupported)
            return false;
        
        currentConfig.useFsbRouting = yes;
        HPET::The()->WriteRegister(COMPARATOR_CAPABILITY_OFFSET(index), currentConfig.squish);
        return true;
    }

    bool HPETComparator::SetIrq(const uint8_t vector)
    {
        if (vector > 32)
            return false; //too big
        if (((1 << vector) & currentConfig.allowedRoutesBitfield) == 0)
            return false; //not allowed

        currentConfig.ioapicRouting = vector;
        HPET::The()->WriteRegister(COMPARATOR_CAPABILITY_OFFSET(index), currentConfig.squish);

        return HPET::The()->ReadRegister(COMPARATOR_CAPABILITY_OFFSET(index)) == currentConfig.squish;
    }

    uint8_t HPETComparator::GetIrq() const
    { return currentConfig.squish; }
    
    HPET hpetInstance;
    HPET* HPET::The()
    {
        return &hpetInstance;
    }

    uint64_t HPET::ReadRegister(uint64_t reg) const
    {
        return sl::MemRead<uint64_t>(baseAddress.raw + reg);
    }

    void HPET::WriteRegister(uint64_t reg, uint64_t value) const
    {
        sl::MemWrite<uint64_t>(baseAddress.raw + reg, value);
    }

    void HPET::Init()
    {
        HPETHeader* hpetHeader = reinterpret_cast<HPETHeader*>(ACPI::The()->FindHeader("HPET"));
        if (hpetHeader == nullptr)
        {
            Log("HPET header not present in ACPI tables.");
            return;
        }

        if (!ACPI::ChecksumValid((SDTHeader*)hpetHeader))
        {
            Log("HPET header corrupted, checksum mismatch. Aborting HPET init.");
            return;
        }

        //NOTE: we're assuming that HPET can only appear in ACPI address space 0 (memory/mmio).
        //This is a fairly safe assumption, as it would be INSANE to try accessing it over pci/ports
        baseAddress = hpetHeader->address.address;
        Log("HPET located at 0x", false);
        Log(sl::UIntToString(baseAddress.raw, BASE_HEX).Data());

        //ensure hpet memory is identity mapped, and physical pages are reserved
        PageTableManager::The()->MapMemory((void*)baseAddress.ptr, (void*)baseAddress.ptr, MemoryMapFlags::WriteAllow | MemoryMapFlags::EternalClaim);
        PageFrameAllocator::The()->LockPage(baseAddress.ptr);

        capabilities.squish = ReadRegister((uint64_t)HPETRegister::GeneralCapabilitiesID);
        capabilities.timersCount++; //NOTE: this value is the highest timer, zero-indexed, so the count is val+1.
        comparatorCount = capabilities.timersCount;

        string format = "HPET capabilities: rev=%u, timers=%u, timerFullWidth=%b, period=0x%x, legacyReplaceSupport=%b";
        Log(sl::FormatToString(0, &format, capabilities.revisionId, capabilities.timersCount, capabilities.largeMainCounter, capabilities.mainCounterPeriod, capabilities.supportsLegacyMapping).Data());

        //get the capabilities of each timer, and stash that for later, then disable all interrrupts
        comparators = reinterpret_cast<HPETComparator*>(KMalloc(sizeof(HPETComparator) * comparatorCount));
        for (size_t i = 0; i < comparatorCount; i++)
        {
            TimerConfigCapabilities defaultCaps;
            defaultCaps.squish = ReadRegister(COMPARATOR_CAPABILITY_OFFSET(i));
            HPETComparator* comparator = new(&comparators[i]) HPETComparator(defaultCaps, i);

            //ensure we're not allowing interrupts, and are defaulting to (a)pic over fsb
            //setup read-only fields (we write their current value back, since default behaviour is not specified)
            uint64_t writeBackValue = 0xffff'ffff'0000'0000 & defaultCaps.squish;
            writeBackValue |= defaultCaps.fsbRoutingSupported ? (1 << 15) : 0;
            writeBackValue |= defaultCaps.largeTimerSize ? (1 << 5) : 0;
            writeBackValue |= defaultCaps.periodicSupported ? (1 << 4) : 0;
            /*  Because all other bits are 0, we're implicitly disabling:
                -interrupts
                -interrupts route to ioapic pin 0 (usually means gsi 0x20)
                -fsb instead of ioapic routing
                -forcing 32bit timer on 64bit main counter
                -accumulator direct write (for periodic mode only)
                -periodic mode
            */
            WriteRegister(COMPARATOR_CAPABILITY_OFFSET(i), writeBackValue);

            format = "HPET comparator %lu config: levelTriggers=%b, 64bitTimer=%b, canFsbRoute=%b, allowedIrqBitmap=0x%lx";
            Log(sl::FormatToString(0, &format, i, comparator->EmitsLevelTriggers(), comparator->FullWidthTimer(), comparator->SupportsFSBRouting(), comparator->GetAllowedRoutingBitmap()).Data());

            //TODO: since only certain ioapic pins are allowed for routing, we should negotiate and reserve them here.
        }

        //enable main counter, disable legacy routing.
        WriteRegister((uint64_t)HPETRegister::GeneralConfig, 0b01);
        Log("HPET enabled, legacy routing disabled, all comparator timer interrupts masked.");
    }

    void HPET::PrintInfo() const
    {
        //TODO: low hanging fruit here
    }

    uint8_t HPET::GetComparatorCount() const
    {
        return comparatorCount;
    }

    uint32_t HPET::GetInterruptStatus() const
    {
        return static_cast<uint32_t>(ReadRegister((uint64_t)HPETRegister::GeneralInterruptStatus));
    }

    HPETComparator* HPET::GetComparator(uint8_t index)
    {
        if (index >= comparatorCount)
            return nullptr;

        return &comparators[index];
    }
}