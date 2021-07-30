#pragma once

#include <stdint-gcc.h>
#include <templates/LinkedList.h>

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
        Version_MaxRedirects = 0x1,
        //read-only, bits 24-27
        ArbitrationPriority = 0x2,

        //read/write, redirect entries are at address pairs (0x10 + 0x11 = int0)
        RedirectionListStart = 0x10,
        RedirectionListEnd = 0x3F,
    };

#define IOAPIC_DELIVERY_MODE_FIXED 0b000
#define IOAPIC_DELIVERY_MODE_LOWEST_PRIORITY 0b001
#define IOAPIC_DELIVERY_MODE_SMI 0b010
#define IOAPIC_DELIVERY_MODE_NMI 0b100
#define IOAPIC_DELIVERY_MODE_INIT 0b101
#define IOAPIC_DELIVERY_MODE_EXT_INT 0b111
#define IOAPIC_DESTINATION_PHYSICAL 0
#define IOAPIC_DESTINATION_LOGICAL 1
//means all irqs have been handled, apic is waiting.
#define IOAPIC_DELIVERY_STATUS_IDLE 0
//means that the irq has been fired, but is still waiting to be processed by the LAPICs.
#define IOAPIC_DELIVERY_STATUS_UNRESOLVED 1
#define IOAPIC_PIN_POLARITY_ACTIVE_HIGH 0
#define IOAPIC_PIN_POLARITY_ACTIVE_LOW 1
#define IOAPIC_TRIGGER_MODE_EDGE 0
#define IOAPIC_TRIGGER_MODE_LEVEL 1
#define IOAPIC_MASK_DISABLE 1
#define IOAPIC_MASK_ENABLE 0

    union IOApicRedirectEntry
    {
        struct 
        {
            uint8_t vector : 8;
            uint8_t deliveryMode : 3;
            uint8_t destinationMode : 1;
            uint8_t deliveryStatus : 1;
            uint8_t pinPolarity : 1;
            uint8_t remoteIRR : 1;
            uint8_t triggerMode : 1;
            uint8_t mask : 1;
            uint64_t reserved : 39;
            uint8_t destination : 8;
        } __attribute__((packed));
        struct
        {
            uint32_t packedUpperHalf;;
            uint32_t packedLowerHalf;
        } __attribute__((packed));
    } __attribute__((packed));

    class IOAPIC
    {
    private:
        uint8_t id;
        uint32_t* baseAddress;
        uint32_t* globalInterruptBase;

    public:
        static void InitAll();

        void Init(uint8_t apicId, uint32_t* address, uint32_t* gsiBase);
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