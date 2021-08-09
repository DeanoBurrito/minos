#pragma once

#include <stdint-gcc.h>

namespace Kernel::Drivers
{
    //Root System Descriptor (v2)
    struct RSDP2
    {
        uint8_t signature[8]; //"RSP PTR "
        uint8_t checksum;
        uint8_t oemId[6];
        uint8_t revision;
        uint32_t rsdtAddress;

        //If revision < 2, anything after this point is junk data.
        uint32_t length;
        uint64_t xsdtAddress;
        uint8_t extendedChecksum;
        uint8_t reserved[3];
    } __attribute__((packed));

    //Standard Descriptor Table Header
    struct SDTHeader
    {
        uint8_t signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        uint8_t oemId[6];
        uint8_t oemTableId[8];
        uint32_t oemRevision;
        uint32_t creatorId;
        uint32_t creatorRevision;
    } __attribute__((packed));

    //"Memory Config" PCI enumeration data, signature "MCFG"
    struct MCFGHeader
    {
        SDTHeader header;
        uint64_t reserved;
    } __attribute__((packed));

#define MADT_FLAGS_DUAL8259_INSTALLED 1 << 1

    //"multiple apic descriptor table", signature "APIC"
    struct MADTHeader
    {
        SDTHeader header;
        uint32_t localAddress;
        uint32_t flags;
    } __attribute__((packed));

    struct MADTEntry
    {
        uint8_t type;
        uint8_t length;
    } __attribute__((packed));

    namespace MADTEntryType
    {
        enum MADTEntryType : uint8_t
        {
            LocalAPIC = 0,
            IOAPIC = 1,
            IOAPIC_SourceOverride = 2,
            IOAPIC_NonMaskableSourceOverride = 3,
            LocalAPIC_NonMaskableInterrupts = 4,
            LocalAPIC_AddressOverride = 5,
            Localx2APIC = 9,
        };
    }

    struct HPETAddressStructure
    {
        uint8_t addressSpaceId; // 0 = memory, 1 = port i/o
        uint8_t registerBitWidth;
        uint8_t registerBitOffset;
        uint8_t reserved;
        uint64_t address;
    } __attribute__((packed));

    //signature is "HPET", high precision/performance event time
    struct HPETHeader
    {
        SDTHeader header;
        uint8_t hardwareRevision;
        uint8_t comparatorCount : 5;
        uint8_t countSize : 1;
        uint8_t reserved : 1;
        uint8_t legacyReplacement : 1;
        uint16_t pciVendorId;
        HPETAddressStructure address;
        uint8_t unitNumber;
        uint16_t minimumTick;
        uint8_t pageProtection;
    } __attribute__((packed));

    struct ACPIGenericAddress
    {
        //0 = system memory, 1 = system IO, 2 = PCI config, 3 = embedded controller, 4 = SMI bus, 5 = CMOS, 6 = PCI BAR target, 
        //7 = IPMI, 8 = GPIO, 9 = serial bus, 0xA = platform communications channel, 0x80+ = OEM specific
        uint8_t addressSpace;
        //for accessing a bitfield
        uint8_t bitWidth;
        //for accessing a bitfield
        uint8_t bitOffset;
        //how many bytes can be read/written at once
        uint8_t accessSize;
        //actual address to use in the defined address space
        uint64_t address;
    } __attribute__((packed));

    //signature is "FACP", fixed ACPI description table - used for power management stuff.
    struct FADTHeader
    {
        SDTHeader header;
        uint32_t firmwareControl; //address of FACS, if zero FACS is unavailable
        uint32_t dsdt;  //address of DSDT, another acpi table detailing system peripherals
        uint8_t reserved; //used in 1.0, best ignored

        uint8_t preferredPowerManagementProfile; //OEM suggested default power manage profile.
        uint16_t sciInterrupt; //system control interrupt: ACPI is telling OS about firmware events. Sharable, level triggered, active low.
        uint32_t smiCommandPort; //system management interrupt: for issuing commands to ACPI firmware. 0 if unsupported.
        uint8_t acpiEnable; //value to write to smiPort for OS to take control of ACPI regs
        uint8_t acpiDisable; //value to return ACPI regs to SMI
        uint8_t s4BiosReq;  //not always supported, write to smiPort to enter alternate S4 state
        uint8_t pStateControl; //value to write to smiPort to take contrl of processor performance state.

        //system port addresses of the following blocks (assuming that means CPU IO?)
        uint32_t pm1aEventBlock;
        uint32_t pm1bEventBlock;
        uint32_t pm1aControlBlock;
        uint32_t pm1bControlBlock;
        uint32_t pm2ControlBlock;
        uint32_t pmTimerBlock;
        uint32_t gpe0Block;
        uint32_t gpe1Block;
        
        //number of bytes decoded by each block at a time.
        uint8_t pm1EventLength; //>= 4 if supported
        uint8_t pm1ControlLength; //>= 2 if supported
        uint8_t pm2ControlLength; //>= 1 if supported
        uint8_t pmTimerLength; //==4 if supported
        uint8_t gpe0Length; //multiple of 2, if supported
        uint8_t gpe1Length; //multiple of 2, if supported
        uint8_t gpe1Base; 

        uint8_t cStateControl; //value to write to smiPort to register support for cstates.
        uint16_t worstC2Latency; //in microseconds, >100 means no hw support 
        uint16_t worstC3Latency; //in microseconds, >1000 means no hw support
        uint16_t flushSize; //for acpi1 cache flushing
        uint16_t flushStride; //for acpi1 cache flushing
        uint8_t dutyOffset; //offset for processor's duty cycle in P_CNT register
        uint8_t dutyWidth; //width in bits, 0 if if feature unsupported

        //RTC register offsets, 0 if unsupported
        uint8_t rtcDayAlarm;
        uint8_t rtcMonthAlarm;
        uint8_t rtcCentury;

        uint16_t iaBootArchitectureFlags; //x86 specific boot flags
        uint8_t reserved2; //must be 0
        uint32_t flags; //fixed feature flags

        ACPIGenericAddress resetRegiser; //offset=0, width=8. 
        uint8_t resetValue; //value to write to resetRegister to reset system.
        uint16_t armBootArchitectureFlags; //ARM specific boot flags
        uint8_t fadtMinorVersion;

        //--- Everything below this line is ACPI 2.0+ only (looking at you QEMU/OVMF) ---
        //These should be treated as overrides to the previously defined values.

        uint64_t x_firmwareControl;
        uint64_t x_dsdt;
        ACPIGenericAddress x_pm1aEventBlock;
        ACPIGenericAddress x_pm1bEventBlock;
        ACPIGenericAddress x_pm1aControlBlock;
        ACPIGenericAddress x_pm1bControlBlock;
        ACPIGenericAddress x_pm2ControlBlock;
        ACPIGenericAddress x_pmTimerBlock;
        ACPIGenericAddress x_gpe0Block;
        ACPIGenericAddress x_gpe1Block;

        ACPIGenericAddress x_sleepControl;
        ACPIGenericAddress x_sleepStatus; 
        uint64_t x_hypervisorId; 
    } __attribute__((packed));
}