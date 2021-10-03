#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Memory.h>
#include <drivers/CPU.h>
#include <arch/x86_64/PageTableDefs.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

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

    void GetPageMapIndices(uint64_t virtualAddress, uint64_t* pdpIndex, uint64_t* pdIndex, uint64_t* ptIndex, uint64_t* pageIndex)
    {
        virtualAddress >>= 12;
        *pageIndex = virtualAddress & 0x1ff;

        virtualAddress >>= 9;
        *ptIndex = virtualAddress & 0x1ff;

        virtualAddress >>= 9;
        *pdIndex = virtualAddress & 0x1ff;

        virtualAddress >>= 9;
        *pdpIndex = virtualAddress & 0x1ff;
    }

    void PageTableManager::Init()
    {
        using namespace Drivers;
        noExecuteSupport = CPU::FeatureSupported(CpuFeatureFlag::NX);
        if (noExecuteSupport)
        {
            uint64_t currentEfer = CPU::ReadMSR(X86_MSR_EFER);
            currentEfer |= 1 << 11;
            CPU::WriteMSR(X86_MSR_EFER, currentEfer);
        }

        //create top level page dir, clear it and map it into it's own space
        topLevelAddr = reinterpret_cast<PageTable*>(PageFrameAllocator::The()->RequestPage());
        MapMemory(topLevelAddr, topLevelAddr, MemoryMapFlags::WriteAllow);
        sl::memset(topLevelAddr, 0, PAGE_SIZE);

        //map the PMM's bitmap as well
        //NOTE: this operates with page-sized chunks, however the bitmap can be way less than that. Possible memory use optimization here.
        uint64_t startAddr = (uint64_t)PageFrameAllocator::The()->pageBitmap.buffer;
        uint64_t endAddr = startAddr + PageFrameAllocator::The()->pageBitmap.size;
        for (sl::UIntPtr bitmapPtr = startAddr; bitmapPtr.raw < endAddr; bitmapPtr.raw += PAGE_SIZE)
            MapMemory(bitmapPtr.ptr, bitmapPtr.ptr, MemoryMapFlags::WriteAllow);

        //map level 4 entry to itself.
        topLevelAddr->entries[511].SetAddress((uint64_t)topLevelAddr >> 12);
    }

    void PageTableManager::MapMemory(void* virtualAddr, MemoryMapFlags flags)
    {
        return MapMemory(virtualAddr, PageFrameAllocator::The()->RequestPage(), flags);
    }

    void PageTableManager::MapMemory(void* virtualAddr, void* physicalAddr, MemoryMapFlags flags)
    {
        uint64_t pd4Index, pd3Index, pd2Index, pageIndex;
        GetPageMapIndices((uint64_t)virtualAddr, &pd4Index, &pd3Index, &pd2Index, &pageIndex);
        PageDirectoryEntry entry;
        PageTable* localTable;
        PageTable* prevTable;

        entry = topLevelAddr->entries[pd4Index];
        if (!entry.GetFlag(PageEntryFlags::Present))
        {
            //allocate a physical page, zero-init it and set required flags
            localTable = reinterpret_cast<PageTable*>(PageFrameAllocator::The()->RequestPage());
            sl::memset(localTable, 0, PAGE_SIZE);

            entry.SetAddress((uint64_t)localTable >> 12);
            entry.SetFlag(PageEntryFlags::Present, true);
            entry.SetFlag(PageEntryFlags::ReadWrite, true);
            topLevelAddr->entries[pd4Index] = entry;
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
        GetPageMapIndices((uint64_t)virtualAddr, &pageDirectoryPtr, &pageDirectory, &pageTable, &page);

        PageDirectoryEntry entry;
        PageTable* childTable;
        void* physicalAddress;

        //chain through each layer of page table, checking if pages are present. If yes, then we unmap it. Otherwise wtf?
        entry = topLevelAddr->entries[pageDirectoryPtr];
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
        physicalAddress = (void*)(entry.GetAddress() << 12);
        entry.value = 0; //set address to 0, and reset all flags to default state
        childTable->entries[page] = entry;

        PageFrameAllocator::The()->FreePage(physicalAddress);
        Drivers::CPU::InvalidatePageTable(virtualAddr); //TODO: this is processor specific, we'll need to sync this up across multiple cores later.
    }

    void PageTableManager::MakeCurrent()
    {
        Drivers::CPU::LoadPageTableMap(topLevelAddr);
    }
}
