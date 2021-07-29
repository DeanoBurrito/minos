#pragma once

#include <stdint-gcc.h>

//MADT doesnt always specify that we need to, but we probably should before enabled LAPIC.
#define FORCE_DISABLE_LEGACY_PIC true

namespace Kernel::Drivers
{
    enum class LocalApicRegisters : uint64_t
    {
        ID = 0x20,
        Version = 0x30,

        TaskPriority = 0x80,
        ArbitrationPriority = 0x90,
        ProcessorPriority = 0xA0,

        EOI = 0xB0,
        RemoteRead = 0xC0,

        LogicalDestination = 0xD0,
        DestinationFormat = 0xE0,
        SpuriousInterruptVector = 0xF0,

        //In-service registers
        ISR0 = 0x100,
        ISR1 = 0x110,
        ISR2 = 0x120,
        ISR3 = 0x130,
        ISR4 = 0x140,
        ISR5 = 0x150,
        ISR6 = 0x160,
        ISR7 = 0x170,

        //Trigger mode registers
        TMR0 = 0x180,
        TMR1 = 0x190,
        TMR2 = 0x1A0,
        TMR3 = 0x1B0,
        TMR4 = 0x1C0,
        TMR5 = 0x1D0,
        TMR6 = 0x1E0,
        TMR7 = 0x1F0,

        //Interrupt request registers
        IRR0 = 0x200,
        IRR1 = 0x210,
        IRR2 = 0x220,
        IRR3 = 0x230,
        IRR4 = 0x240,
        IRR5 = 0x250,
        IRR6 = 0x260,
        IRR7 = 0x270,

        ErrorStatus = 0x280,
        LvtCorrectedMachineCheckInterrupt = 0x2F0,
        InterruptCommand0 = 0x300,
        InterruptCommand1 = 0x310,

        LvtTimer = 0x320,
        LvtThermalSensor = 0x330,
        LvtPerformanceMonitoringCounters = 0x340,
        LvtLINT0 = 0x350,
        LvtLINT1 = 0x360,
        LvtError = 0x370,

        TimerInitialCount = 0x380,
        TimerCurrentCount = 0x390,
        TimerDivideConfig = 0x3E0,
    };
    
    class APIC
    {
    private:
        uint32_t localApicAddr;

    public:
        static APIC* Local();

        void Init();
        void PrintTablesInfo();
        uint64_t GetLocalBase(); //useful for identity mapping and locking.
    };
}