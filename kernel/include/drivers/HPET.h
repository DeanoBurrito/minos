#pragma once

#include <stdint-gcc.h>

namespace Kernel::Drivers
{
    //each are 64 bits wide
    enum class HPETRegister : uint64_t
    {
        GeneralCapabilitiesID = 0x00,
        GeneralConfig = 0x10,
        GeneralInterruptStatus = 0x20,
        MainCounterValue = 0xF0,
    };

    union GeneralCapabilitiesID
    {
        uint64_t squish;
        struct 
        {
            //must not be 0
            uint8_t revisionId;
            //number of timers (these are 0 indexed)
            uint8_t timersCount : 5; 
            //true if main counter is 64 bits
            bool largeMainCounter : 1;
            uint8_t reserved : 1;
            bool supportsLegacyMapping : 1;
            //should be interpreted similarly to PCI vendor ID
            uint16_t vendorId;
            //period of main counter tick in femtoseconds. Must be > 0 and <= 100 nanoseconds (0x5f5e100)
            uint32_t mainCounterPeriod;
        } __attribute__((packed));
    };
    
    /*  Other main registers registers:
            These have so few fields i'm not bothering with structs for them.
        GeneralConfig field:
            bit 0 - set to enable main counter and interrupts
            bit 1 - set to enable legacy routing (timer 0 -> PIT, timer 1 -> RTC)
        GneralInterruptStatus:
            bit n (where n is timer number) - if timer is level triggered, this is set until 1 is written to that bit.
    */

    union TimerConfigCapabilities
    {
        uint64_t squish;
        struct
        {
            uint8_t reserved0 : 1;
            //false if generates edge triggers interrupts
            bool generatesLevelTriggers : 1;
            bool enable : 1;
            //if false, timer operates in one-shot mode, ignored if periodic mode is unsupported.
            bool periodicMode : 1;
            bool periodicSupported : 1;
            //if set, timer is 64bits otherwise its 32bit
            bool largeTimerSize : 1;
            //if set and timer is in periodic mode, enables writing directly to accumulator. Automatically clears on write
            bool setAccumulator : 1;
            uint8_t reserved1 : 1;
            //if set 64bit timer will be forced to run in 32bit mode, otherwise ignored.
            bool forceSmallTimer : 1;
            //read back after write to ensure we wrote a valid value
            uint8_t ioapicRouting : 5;
            //if set timer will send interrupts over fsb, ignoring the apic
            bool useFsbRouting : 1;
            bool fsbRoutingSupported : 1;
            uint16_t reserved2;
            //each set bit means we can route to that irq line of an ioapic
            uint32_t allowedRoutesBitfield;
        } __attribute__((packed));
    };

#define COMPARATOR_CAPABILITY_OFFSET(n) (0x100 + 0x20 * (n))
#define COMPARATOR_VALUE_OFFSET(n) (0x108 + 0x20 * (n))
#define COMPARATOR_FSB_ROUTE(n) (0x110 + 0x20 * (n))
    
    class HPET
    {
    private:
        uint64_t comparatorCount;
        GeneralCapabilitiesID capabilities;
        uint64_t baseAddress;

        uint64_t ReadRegister(uint64_t reg);
        void WriteRegister(uint64_t reg, uint64_t value);

    public:
        static HPET* The();

        void Init();
        void PrintInfo();
        uint8_t GetTimerCount();

        TimerConfigCapabilities GetTimerCapabilities(uint8_t index);
        void SetTimerCapabilities(uint8_t index, TimerConfigCapabilities caps);
        void SetTimerValue(uint8_t index, uint64_t femtoseconds, bool makeRelative = false);
    };
}