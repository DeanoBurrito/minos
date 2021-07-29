#include <KLog.h>
#include <StringUtil.h>
#include <drivers/ACPI.h>
#include <memory/Utilities.h>

namespace Kernel::Drivers
{
    ACPI acpiInstance;
    ACPI* ACPI::The()
    {
        return &acpiInstance;
    }

    void ACPI::Init(void* rsdPtr)
    {
        //NOTE: not checking for RSDP1 here as it only seems to appear in the earliest of early specs. Compatability issues should occur from other areas first.
        rsdp = reinterpret_cast<RSDP2*>(rsdPtr);
        if (rsdp->revision < 2)
        {
            Log("ACPI subsystem using revision 1. RSDP with 32bit pointers.");
            rootHeader = reinterpret_cast<SDTHeader*>(rsdp->rsdtAddress);
            revisionPtrSize = 4;
        }
        else
        {
            Log("ACPI subsystem using revision 2+. XSDT with 64bit pointers.");
            rootHeader = reinterpret_cast<SDTHeader*>(rsdp->xsdtAddress);
            revisionPtrSize = 8;
        }

        Log("ACPI subsystem intializing with RSDP=0x", false);
        Log(ToStrHex((uint64_t)rsdp));
        PrintTables();

        //https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/
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
        memcopy(header->signature, reusableBuffer, 4);
        reusableBuffer[4] = 0;
        Log(reusableBuffer);

        Log("  |- Length: 0x", false);
        Log(ToStrHex(header->length));

        Log("  |- Revision: ", false);
        Log(ToStr(header->revision));

        Log("  |- Checksum: 0x", false);
        Log(ToStrHex(header->checksum));

        Log("  |- OEM ID: ", false);
        memcopy(header->oemId, reusableBuffer, 6);
        reusableBuffer[6] = 0;
        Log(reusableBuffer);

        Log("  |- OEM Table ID: ", false);
        memcopy(header->oemTableId, reusableBuffer, 8);
        reusableBuffer[8] = 0;
        Log(reusableBuffer);

        Log("  |- OEM Revision: ", false);
        Log(ToStr(header->oemRevision));

        Log("  |- Creator ID: 0x", false);
        Log(ToStrHex(header->creatorId));

        Log("  |- Creator Revision: ", false);
        Log(ToStr(header->creatorRevision));

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
        
        memcopy(rsdp->signature, printableSig, 8);
        printableSig[8] = 0;
        Log("  |- Signature: ", false);
        Log(printableSig);
        
        Log("  |- Checksum: 0x", false);
        Log(ToStrHex(rsdp->checksum));

        Log("  |- OEM ID: ", false);
        memcopy(rsdp->oemId, printableSig, 6);
        printableSig[6] = 0;
        Log(printableSig);

        Log("  |- Revision: ", false);
        Log(ToStr(rsdp->revision));

        Log("  |- RSDT addr: 0x", false);
        Log(ToStrHex(rsdp->rsdtAddress));

        Log("  |- Length: 0x", false);
        Log(ToStrHex(rsdp->length));

        Log("  |- XSDT addr: 0x", false);
        Log(ToStrHex(rsdp->xsdtAddress));

        Log("  |- Ext. Checksum: 0x", false);
        Log(ToStrHex(rsdp->extendedChecksum));

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
            
            if (memcmp(localHeader->signature, signature, 4) == 0)
            {
                //it's a match!
                return localHeader;
            }
        }
        
        return nullptr;
    }
}