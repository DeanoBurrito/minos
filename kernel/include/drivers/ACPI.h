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
    
    class ACPI
    {
    private:
        RSDP2* rsdp;
        SDTHeader* rootHeader; //set to xsdt if its available, otherwise falls back to rsdt
        int revisionPtrSize;

        void PrintSDT(SDTHeader* header, char* reusableBuffer);

    public:
        static ACPI* The();
  
        void Init(void* rsdp);
        void PrintTables();
        //Searches ACPI tables for a header with matching sig, returns nullptr if not found.
        SDTHeader* FindHeader(const char* const signature);
        //Searches for a matching header, starting a certain point
        SDTHeader* FindNextHeader(const char* const signature, const SDTHeader* const start);
    };
}