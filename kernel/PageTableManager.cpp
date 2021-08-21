#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Memory.h>
#include <drivers/CPU.h>

#define X86_MSR_EFER 0xC0000080

namespace Kernel
{   
    MemoryMapFlags operator&(MemoryMapFlags a, MemoryMapFlags b)
    {
        return static_cast<MemoryMapFlags>((uint64_t)a & (uint64_t)b);
    }

    MemoryMapFlags operator|(MemoryMapFlags a, MemoryMapFlags b)
    {
        return static_cast<MemoryMapFlags>((uint64_t)a | (uint64_t)b);
    }
    
    void PageDirectoryEntry::SetFlag(PageEntryFlags flag, bool enabled)
    {
        uint64_t mask = (uint64_t)1 << (uint64_t)flag;
        value &= ~mask;
        if (enabled)
            value |= mask;
    }

    bool PageDirectoryEntry::GetFlag(PageEntryFlags flag)
    {
        uint64_t mask = (uint64_t)1 << (uint64_t)flag;
        return (value & mask) > 0 ? true : false;
    }

    void PageDirectoryEntry::SetAddress(uint64_t address)
    {
        address &= 0x000000FFFFFFFFFF;
        value &= 0xFFF0000000000FFF;
        value |= (address << 12);
    }

    uint64_t PageDirectoryEntry::GetAddress()
    {
        return (value & 0x000FFFFFFFFFF000) >> 12;
    }

    PageTableManager globalPageTableManager;
    PageTableManager* PageTableManager::The()
    {
        return &globalPageTableManager;
    }

    void PageTableManager::Init(PageTable* pml4Address)
    {
        pml4Addr = pml4Address;

        using namespace Drivers;
        noExecuteSupport = CPU::FeatureSupported(CpuFeatureFlag::NX);
        if (noExecuteSupport)
        {
            uint64_t currentEfer = CPU::ReadMSR(X86_MSR_EFER);
            currentEfer |= 1 << 11;
            CPU::WriteMSR(X86_MSR_EFER, currentEfer);
        }
    }

    void PageTableManager::MapMemory(void* virtualAddr, MemoryMapFlags flags)
    {
        return MapMemory(virtualAddr, PageFrameAllocator::The()->RequestPage(), flags);
    }

    void PageTableManager::MapMemory(void* virtualAddr, void* physicalAddr, MemoryMapFlags flags)
    {
        uint64_t pd4Index, pd3Index, pd2Index, pageIndex;
        PageFrameAllocator::GetPageMapIndices((uint64_t)virtualAddr, &pd4Index, &pd3Index, &pd2Index, &pageIndex);
        PageDirectoryEntry entry;
        PageTable* localTable;
        PageTable* prevTable;

        entry = pml4Addr->entries[pd4Index];
        if (!entry.GetFlag(PageEntryFlags::Present))
        {
            //allocate a physical page, zero-init it and set required flags
            localTable = reinterpret_cast<PageTable*>(PageFrameAllocator::The()->RequestPage());
            sl::memset(localTable, 0, PAGE_SIZE);

            entry.SetAddress((uint64_t)localTable >> 12);
            entry.SetFlag(PageEntryFlags::Present, true);
            entry.SetFlag(PageEntryFlags::ReadWrite, true);
            pml4Addr->entries[pd4Index] = entry;
        }
        else
            localTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);

        prevTable = localTable;
        entry = localTable->entries[pd3Index];
        if (!entry.GetFlag(PageEntryFlags::Present))
        {
            //same as above
            localTable = reinterpret_cast<PageTable*>(PageFrameAllocator::The()->RequestPage());
            sl::memset(localTable, 0, PAGE_SIZE);

            entry.SetAddress((uint64_t)localTable >> 12);
            entry.SetFlag(PageEntryFlags::Present, true);
            entry.SetFlag(PageEntryFlags::ReadWrite, true);
            prevTable->entries[pd3Index] = entry;
        }
        else
            localTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);

        prevTable = localTable;
        entry = localTable->entries[pd2Index];
        if (!entry.GetFlag(PageEntryFlags::Present))
        {
            localTable = reinterpret_cast<PageTable*>(PageFrameAllocator::The()->RequestPage());
            sl::memset(localTable, 0, PAGE_SIZE);

            entry.SetAddress((uint64_t)localTable >> 12);
            entry.SetFlag(PageEntryFlags::Present, true);
            entry.SetFlag(PageEntryFlags::ReadWrite, true);
            prevTable->entries[pd2Index] = entry;
        }
        else
            localTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);
    
        //TODO: lazy allocation/ImmediateAlloc flag.
        //since we dont know where the physical address came from, make sure it dosnt get re-assigned.
        // if ((flags & MemoryMapFlags::EternalClaim) != MemoryMapFlags::None)
        //     PageFrameAllocator::The()->ReservePage(physicalAddr);
        // else
        //     PageFrameAllocator::The()->LockPage(physicalAddr);
        
        entry = localTable->entries[pageIndex];
        entry.SetAddress((uint64_t)physicalAddr >> 12);
        entry.SetFlag(PageEntryFlags::Present, true);

        //set any requested flags
        if ((flags & MemoryMapFlags::WriteAllow) != MemoryMapFlags::None)
            entry.SetFlag(PageEntryFlags::ReadWrite, true);
        if ((flags & MemoryMapFlags::ExecuteAllow) == MemoryMapFlags::None && noExecuteSupport)
            entry.SetFlag(PageEntryFlags::NoExecute, true);
        if ((flags & MemoryMapFlags::DisableCache) != MemoryMapFlags::None)
            entry.SetFlag(PageEntryFlags::CacheDisabled, true);
        if ((flags & MemoryMapFlags::DisablePrivChecks) == MemoryMapFlags::None)
            entry.SetFlag(PageEntryFlags::SuperOnly, true);

        //update values in page table
        localTable->entries[pageIndex] = entry;
    }

    void PageTableManager::UnmapMemory(void* virtualAddr)
    {
        uint64_t pageDirectoryPtr, pageDirectory, pageTable, page;
        PageFrameAllocator::GetPageMapIndices((uint64_t)virtualAddr, &pageDirectoryPtr, &pageDirectory, &pageTable, &page);

        PageDirectoryEntry entry;
        PageTable* childTable;
        void* physicalAddress;

        //chain through each layer of page table, checking if pages are present. If yes, then we unmap it. Otherwise wtf?
        entry = pml4Addr->entries[pageDirectoryPtr];
        if (!entry.GetFlag(PageEntryFlags::Present))
            return; //was never mapped in the first place
        childTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);

        entry = childTable->entries[pageDirectory];
        if (!entry.GetFlag(PageEntryFlags::Present))
            return;
        childTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);

        entry = childTable->entries[pageTable];
        if (!entry.GetFlag(PageEntryFlags::Present))
            return;
        childTable = reinterpret_cast<PageTable*>((uint64_t)entry.GetAddress() << 12);

        entry = childTable->entries[page];
        if (!entry.GetFlag(PageEntryFlags::Present))
            return;

        //clear flag, place back in page table, and then release physical page
        entry.SetFlag(PageEntryFlags::Present, false);
        physicalAddress = (void*)(entry.GetAddress() << 12);
        entry.SetAddress(0);
        childTable->entries[page] = entry;

        PageFrameAllocator::The()->FreePage(physicalAddress);
        Drivers::CPU::InvalidatePageTable(childTable); //TODO: this is processor specific, we'll need to sync this up across multiple cores later.
    }

    void PageTableManager::MakeCurrentMap()
    {
        Drivers::CPU::LoadPageTableMap(pml4Addr);
    }
}
