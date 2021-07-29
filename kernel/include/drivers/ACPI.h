#pragma once

#include <stdint-gcc.h>
#include <drivers/ACPIHeaders.h>

namespace Kernel::Drivers
{
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