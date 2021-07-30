#pragma once

#include <stdint-gcc.h>

//MADT doesnt always specify that we need to, but we probably should before enabled LAPIC.
#define FORCE_DISABLE_LEGACY_PIC true
#define APIC_BASE_MSR 0x1B

namespace Kernel::Drivers
{
    enum class LocalApicRegisters : uint64_t
    {
        ID = 0x2,
        Version = 0x3,

        TaskPriority = 0x8,
        ArbitrationPriority = 0x9,
        ProcessorPriority = 0xA,

        EOI = 0xB,
        RemoteRead = 0xC,

        LogicalDestination = 0xD,
        DestinationFormat = 0xE,
        SpuriousInterruptVector = 0xF,

        //In-service registers
        ISR0 = 0x10,
        ISR1 = 0x11,
        ISR2 = 0x12,
        ISR3 = 0x13,
        ISR4 = 0x14,
        ISR5 = 0x15,
        ISR6 = 0x16,
        ISR7 = 0x17,

        //Trigger mode registers
        TMR0 = 0x18,
        TMR1 = 0x19,
        TMR2 = 0x1A,
        TMR3 = 0x1B,
        TMR4 = 0x1C,
        TMR5 = 0x1D,
        TMR6 = 0x1E,
        TMR7 = 0x1F,

        //Interrupt request registers
        IRR0 = 0x20,
        IRR1 = 0x21,
        IRR2 = 0x22,
        IRR3 = 0x23,
        IRR4 = 0x24,
        IRR5 = 0x25,
        IRR6 = 0x26,
        IRR7 = 0x27,

        ErrorStatus = 0x28,
        LvtCorrectedMachineCheckInterrupt = 0x2F,
        InterruptCommand0 = 0x30,
        InterruptCommand1 = 0x31,

        LvtTimer = 0x32,
        LvtThermalSensor = 0x33,
        LvtPerformanceMonitoringCounters = 0x34,
        LvtLINT0 = 0x35,
        LvtLINT1 = 0x36,
        LvtError = 0x37,

        TimerInitialCount = 0x38,
        TimerCurrentCount = 0x39,
        TimerDivideConfig = 0x3E,
    };

    enum class IOApicRegisters : uint64_t
    {
        //read/write the apic's id (bits 24-27 only, rest are ignored)
        ID = 0x0,
        //bits 0-7 contain revision, max redirection entries count in 16-23
        GetDetails = 0x1,
        //read-only, bits 24-27
        ArbitrationPriority = 0x2,

        //read/write, redirect entries are at address pairs (0x10 + 0x11)
        RedirectionListStart = 0x10,
        RedirectionListEnd = 0x3F,
    };
    
    class APIC
    {
    private:
        uint32_t* localApicAddr;

        void SetLocalBase(uint64_t addr);
        uint32_t ReadRegister(LocalApicRegisters reg);
        void WriteRegister(LocalApicRegisters reg, uint32_t value);

    public:
        static APIC* Local();

        void Init();
        void PrintTablesInfo();
        uint64_t GetLocalBase(); //useful for identity mapping and locking.

        void SendEOI();
        void StartTimer(uint8_t interruptVectorNumber);
    };
}