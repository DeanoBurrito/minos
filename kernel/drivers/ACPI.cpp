#include <KLog.h>
#include <StringExtras.h>
#include <drivers/ACPI.h>
#include <Memory.h>
#include <stddef.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Platform.h>

namespace Kernel::Drivers
{
    ACPI acpiInstance;
    ACPI* ACPI::The()
    {
        return &acpiInstance;
    }

    bool ACPI::ChecksumValid(SDTHeader* header)
    {
        uint64_t checksum = 0;
        for (size_t i = 0; i < header->length; i++)
            checksum += (reinterpret_cast<uint8_t*>(header))[i];
        
        return (checksum & 0xFF) == 0;
    }

    void ACPI::Init(void* rsdPtr)
    {
        //NOTE: not checking for RSDP1 here as it only seems to appear in the earliest of early specs. Compatability issues should occur from other areas first.
        rsdp = reinterpret_cast<RSDP2*>(rsdPtr);
        uint64_t tableSize = 0;

        if (rsdp->revision < 2)
        {
            Log("ACPI subsystem using revision 1. RSDP with 32bit pointers.");
            rootHeader = reinterpret_cast<SDTHeader*>(rsdp->rsdtAddress);
            revisionPtrSize = 4;

            //verify checksum is correct
            uint64_t checksum = 0;
            for (int i = 0; i < 20; i++) //20 bytes is size of rsdp v1 header
                checksum += ((uint8_t*)rsdPtr)[i];
            if ((checksum & 0xFF) != 0)
                LogError("RSDP checksum mismatch! Init will continue, but this should not be ignored.");

            tableSize = 20 + rootHeader->length;
        }
        else
        {
            Log("ACPI subsystem using revision 2+. XSDT with 64bit pointers.");
            rootHeader = reinterpret_cast<SDTHeader*>(rsdp->xsdtAddress);
            revisionPtrSize = 8;

            uint64_t checksum = 0;
            for (size_t i = 0; i < sizeof(RSDP2); i++) 
                checksum += ((uint8_t*)rsdPtr)[i];
            if ((checksum & 0xFF) != 0)
                LogError("RSDP checksum mismatch! Init will continue, but this should not be ignored.");
            
            tableSize = rsdp->length;
        }

        //TODO: ensure tables are mapped into VM and physical pages locked.

        Log("ACPI subsystem intializing with RSDP=0x", false);
        Log(sl::UIntToString((uint64_t)rsdp, BASE_HEX).Data());

        //PrintTables();
    }

    void ACPI::PrintSDT(SDTHeader* header, char* reusableBuffer)
    {
        if (header == nullptr)
        {
            Log("-- [SDT Header = nullptr] ---");
            return;
        }
        
        Log("--[ SDT Header ]--");
        Log("  |- Signature: ", false);
        sl::memcopy(header->signature, reusableBuffer, 4);
        reusableBuffer[4] = 0;
        Log(reusableBuffer);

        Log("  |- Length: 0x", false);
        Log(sl::UIntToString(header->length, BASE_HEX).Data());

        Log("  |- Revision: ", false);
        Log(sl::UIntToString(header->revision, BASE_DECIMAL).Data());

        Log("  |- Checksum: 0x", false);
        Log(sl::UIntToString(header->checksum, BASE_HEX).Data());

        Log("  |- OEM ID: ", false);
        sl::memcopy(header->oemId, reusableBuffer, 6);
        reusableBuffer[6] = 0;
        Log(reusableBuffer);

        Log("  |- OEM Table ID: ", false);
        sl::memcopy(header->oemTableId, reusableBuffer, 8);
        reusableBuffer[8] = 0;
        Log(reusableBuffer);

        Log("  |- OEM Revision: ", false);
        Log(sl::UIntToString(header->oemRevision, BASE_DECIMAL).Data());

        Log("  |- Creator ID: 0x", false);
        Log(sl::UIntToString(header->creatorId, BASE_HEX).Data());

        Log("  |- Creator Revision: ", false);
        Log(sl::UIntToString(header->creatorRevision, BASE_DECIMAL).Data());

        Log("--[ End Header ]--");
    }

    void ACPI::PrintTables()
    {
        if (rsdp == nullptr)
            return;

        char printableSig[9] 
        { 0, 0, 0, 0, 
          0, 0, 0, 0, 0}; 
        Log("ACPI Table Dump:");
        Log("--[ RSDP ]--");
        
        sl::memcopy(rsdp->signature, printableSig, 8);
        printableSig[8] = 0;
        Log("  |- Signature: ", false);
        Log(printableSig);
        
        Log("  |- Checksum: 0x", false);
        Log(sl::UIntToString(rsdp->checksum, BASE_HEX).Data());

        Log("  |- OEM ID: ", false);
        sl::memcopy(rsdp->oemId, printableSig, 6);
        printableSig[6] = 0;
        Log(printableSig);

        Log("  |- Revision: ", false);
        Log(sl::UIntToString(rsdp->revision, BASE_DECIMAL).Data());

        Log("  |- RSDT addr: 0x", false);
        Log(sl::UIntToString(rsdp->rsdtAddress, BASE_HEX).Data());

        Log("  |- Length: 0x", false);
        Log(sl::UIntToString(rsdp->length, BASE_HEX).Data());

        Log("  |- XSDT addr: 0x", false);
        Log(sl::UIntToString(rsdp->xsdtAddress, BASE_HEX).Data());

        Log("  |- Ext. Checksum: 0x", false);
        Log(sl::UIntToString(rsdp->extendedChecksum, BASE_HEX).Data());

        Log("--[ End RSDP ]--");

        int entriesCount = (rootHeader->length - sizeof(SDTHeader)) / 4;

        //print all the available tables
        for (int i = 0; i < entriesCount; i++)
        {
            SDTHeader* localHeader; 
            if (rsdp->revision < 2)
                localHeader = (SDTHeader*)*(uint32_t*)((uint64_t)rootHeader + sizeof(SDTHeader) + (i * revisionPtrSize));
            else
                localHeader = (SDTHeader*)*(uint64_t*)((uint64_t)rootHeader + sizeof(SDTHeader) + (i * revisionPtrSize));
            char buff[9];
            PrintSDT(localHeader, buff);
        }
    }

    SDTHeader* ACPI::FindHeader(const char* const signature)
    {
        return FindNextHeader(signature, nullptr);
    }
    
    SDTHeader* ACPI::FindNextHeader(const char* const signature, const SDTHeader* const start)
    {
        int entriesCount = (rootHeader->length - sizeof(SDTHeader)) / revisionPtrSize;
        bool foundStart = false;
        if (start == nullptr)
            foundStart = true;
        
        for (int i = 0; i < entriesCount; i++)
        {
            SDTHeader* localHeader; //getting this depends on which revision of the ACPI spec we've been supplied with.
            if (rsdp->revision < 2)
                localHeader = (SDTHeader*)*(uint32_t*)((uint64_t)rootHeader + sizeof(SDTHeader) + (i * revisionPtrSize));
            else
                localHeader = (SDTHeader*)*(uint64_t*)((uint64_t)rootHeader + sizeof(SDTHeader) + (i * revisionPtrSize));

            //before we return ANY results, check if we're after the starting header
            if (!foundStart)
            {
                if (localHeader == start)
                    foundStart = true;
                continue;
            }

            //if sig is empty, just return the first header
            if (signature == nullptr)
                return localHeader;
            
            if (sl::memcmp(localHeader->signature, signature, 4) == 0)
            {
                //it's a match!
                return localHeader;
            }
        }
        
        return nullptr;
    }
}