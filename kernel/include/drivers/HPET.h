#pragma once

#include <stdint.h>
#include <Memory.h>

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

    class HPET;

    //friendly wrapper around a hpet dev
    class HPETComparator
    {
    friend HPET;
    private:
        const uint8_t index;
        const TimerConfigCapabilities capabilities; //TODO: replace bitfield with plain uint, since bitfields are unreliable
        TimerConfigCapabilities currentConfig;

        HPETComparator(const TimerConfigCapabilities caps, const uint8_t id);

    public:
        uint32_t GetAllowedRoutingBitmap() const;
        bool IsEnabled() const;
        bool HasPeriodicMode() const;
        bool EmitsLevelTriggers() const;
        bool FullWidthTimer() const;
        bool SupportsFSBRouting() const;

        bool EnableFSBRouting(const bool yes = true);
        //this directs the comparator to trigger this irq, ioapic will still need to unmask this.
        bool SetIrq(const uint8_t vector);
        uint8_t GetIrq() const;
    };
    
    class HPET
    {
    friend HPETComparator;
    private:
        uint64_t comparatorCount;
        GeneralCapabilitiesID capabilities;
        sl::UIntPtr baseAddress;
        HPETComparator* comparators;

        uint64_t ReadRegister(uint64_t reg) const;
        void WriteRegister(uint64_t reg, uint64_t value) const;

    public:
        static HPET* The();

        void Init();
        void PrintInfo() const;
        uint8_t GetComparatorCount() const;
        uint32_t GetInterruptStatus() const;

        [[nodiscard]]
        HPETComparator* GetComparator(uint8_t index = 0);
    };
}