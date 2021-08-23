#include <drivers/HPET.h>
#include <drivers/ACPI.h>
#include <PageTableManager.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Formatting.h>

namespace Kernel::Drivers
{
    HPET hpetInstance;
    HPET* HPET::The()
    {
        return &hpetInstance;
    }

    uint64_t HPET::ReadRegister(uint64_t reg)
    {
        return *(uint64_t*)(baseAddress + reg);
    }

    void HPET::WriteRegister(uint64_t reg, uint64_t value)
    {
        *(uint64_t*)(baseAddress + reg) = value;
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
        Log(sl::UIntToString(baseAddress, BASE_HEX).Data());

        //ensure hpet memory is identity mapped
        PageTableManager::The()->MapMemory((void*)baseAddress, (void*)baseAddress, MemoryMapFlags::WriteAllow | MemoryMapFlags::EternalClaim);

        capabilities.squish = ReadRegister((uint64_t)HPETRegister::GeneralCapabilitiesID);
        capabilities.timersCount++; //NOTE: this value is the highest timer, zero-indexed, so the count is val+1.
        comparatorCount = capabilities.timersCount;

        string format = "HPET capabilities: rev=%u, timers=%u, main_counter64=%u, period=0x%x";
        Log(sl::FormatToString(0, &format, capabilities.revisionId, capabilities.timersCount, capabilities.largeMainCounter, capabilities.mainCounterPeriod).Data());

        //TODO: since timers are only allowed certain routing, we should try and reserve those here.

        //enable main counter, disable legacy routing.
        WriteRegister((uint64_t)HPETRegister::GeneralConfig, 0b01);
    }

    void HPET::PrintInfo()
    {
        //TODO: low hanging fruit here
    }

    uint8_t HPET::GetTimerCount()
    {
        return comparatorCount;
    }

    TimerConfigCapabilities HPET::GetTimerCapabilities(uint8_t index)
    {
        if (index >= capabilities.timersCount)
            return TimerConfigCapabilities();
        
        TimerConfigCapabilities caps;
        caps.squish = ReadRegister(COMPARATOR_CAPABILITY_OFFSET(index));
        return caps;
    }

    void HPET::SetTimerCapabilities(uint8_t index, TimerConfigCapabilities caps)
    {
        if (index >= capabilities.timersCount)
            return;

        WriteRegister(COMPARATOR_CAPABILITY_OFFSET(index), caps.squish);
    }

    void HPET::SetTimerValue(uint8_t index, uint64_t femtoseconds, bool makeRelative)
    {
        if (index >= capabilities.timersCount)
            return;

        if (makeRelative)
        {
            uint64_t mainCounter = ReadRegister((uint64_t)HPETRegister::MainCounterValue);
            WriteRegister(COMPARATOR_VALUE_OFFSET(index), femtoseconds + mainCounter);
        }
        else
            WriteRegister(COMPARATOR_VALUE_OFFSET(index), femtoseconds);
    }
}