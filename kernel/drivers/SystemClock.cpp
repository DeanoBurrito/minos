#include <drivers/SystemClock.h>
#include <drivers/CPU.h>
#include <DateTime.h>
#include <drivers/ACPI.h>
#include <drivers/CmosRTC.h>
#include <drivers/APIC.h>
#include <drivers/8253PIT.h>
#include <drivers/HPET.h>
#include <KLog.h>

namespace Kernel::Drivers
{
    void SystemClock::Init()
    {
        //enumerate all possible system clocks, initialize them and do any mapping required (if HPET is available, disable PIT).
        //Dont forget to set RTC century register if it's available
    }

    bool SystemClock::SourceAvailable(ClockSourceType type)
    {
        switch (type)
        {
        case ClockSourceType::X86_TSC:
            return CPU::FeatureSupported(CpuFeatureFlag::TSC);

        case ClockSourceType::X86_LAPIC:
            return CPU::FeatureSupported(CpuFeatureFlag::APIC); //NOTE: All apics support both single-shot/periodic modes, no need to check for that

        case ClockSourceType::X86_LAPIC_TSC:
            return CPU::FeatureSupported(CpuFeatureFlag::APIC) && CPU::FeatureSupported(CpuFeatureFlag::TSC);

        case ClockSourceType::X86_PIT:
            return true; //Always supported, even if now usually emulated by HPET

        case ClockSourceType::X86_HPET:
            return ACPI::The()->FindHeader("HPET") != nullptr; //TODO: should probably query the HPET driver itself, rather than consult ACPI tables (init might have failed)

        //coming soon(tm) with the arm version
        case ClockSourceType::ARM_GENERIC_TIMER:
        case ClockSourceType::ARM_LOCAL_TIMER:
        case ClockSourceType::ARM_SYSTEM_TIMER:
            return false;
            
        default:
            return false;
        }
    }

    void SystemClock::SelectPrimarySource(ClockSourceType type)
    {
        //TODO: set DateTime::ticksPerSecond based on this
        switch (type)
        {
        case ClockSourceType::X86_TSC:
            return;
        case ClockSourceType::X86_LAPIC:
            return;
        case ClockSourceType::X86_LAPIC_TSC:
            return;
        case ClockSourceType::X86_HPET:
            return;
        case ClockSourceType::X86_PIT:
            return;

        default:
            Log("Unsupported clock source for system primary");
            return;
        }
    }

    size_t SystemClock::GetSourceTimers(ClockSourceType type)
    {
        switch (type)
        {
        case ClockSourceType::X86_TSC:
            return 0;
        case ClockSourceType::X86_LAPIC:
            return 1;
        case ClockSourceType::X86_LAPIC_TSC:
            return 1;
        case ClockSourceType::X86_PIT:
            return 1;
        case ClockSourceType::X86_HPET:
            return 3; //minimum of 3, should be checking HPET driver really TODO:

        default:
            return 0;
        }
    }

    bool SystemClock::SetupSingleShotIRQ(ClockSourceType type, size_t timerIndex, uint8_t vector, uint64_t millis)
    {
        switch (type)
        {
        case ClockSourceType::X86_LAPIC:
            return false;
        case ClockSourceType::X86_LAPIC_TSC:
            return false;
        case ClockSourceType::X86_HPET:
            return false;
        case ClockSourceType::X86_PIT:
            return false;

        default:
            Log("Unsupported ClockSourceType for single shot IRQ.");
            return false;
        }
    }

    bool SystemClock::SetupTimerIRQ(ClockSourceType type, size_t timerIndex, uint8_t vector, uint64_t millis)
    {
        switch (type)
        {
        case ClockSourceType::X86_LAPIC:
            APIC::Local()->StartTimer(vector);
            return true;
        
        case ClockSourceType::X86_PIT:
            return false; //TODO: setup PIT and adjust IOAPIC entry to match requested vector
        case ClockSourceType::X86_HPET:
            return false; //TODO: setup HPET and adjust IOAPIC entries

        default:
            Log("Unsupported ClockSourceType for periodic timer IRQ.");
            return false;
        }
    }

}