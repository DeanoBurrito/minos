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

    //"Memory Config" PCI enumeration data
    struct MCFGHeader
    {
        SDTHeader header;
        uint64_t reserved;
    } __attribute__((packed));

#define MADT_FLAGS_DUAL8259_INSTALLED 1 << 1

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
}