#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Memory.h>
#include <drivers/CPU.h>

namespace Kernel
{
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
    }

    void PageTableManager::MapMemory(void* virtualAddr, void* physicalAddr)
    {
        uint64_t pageDirectoryPtr, pageDirectory, pageTable, page;
        PageFrameAllocator::GetPageMapIndices((uint64_t)virtualAddr, &pageDirectoryPtr, &pageDirectory, &pageTable, &page);
        PageDirectoryEntry pageEntry;

        //Getting page directory pointer
        pageEntry = pml4Addr->entries[pageDirectoryPtr];
        PageTable* pdp;
        if (!pageEntry.GetFlag(PageEntryFlags::Present))
        {
            pdp = (PageTable*)PageFrameAllocator::The()->RequestPage();
            sl::memset(pdp, 0, PAGE_SIZE);

            pageEntry.SetAddress((uint64_t)pdp >> 12);
            pageEntry.SetFlag(PageEntryFlags::Present, true);
            pageEntry.SetFlag(PageEntryFlags::ReadWrite, true);
            pml4Addr->entries[pageDirectoryPtr] = pageEntry;
        }
        else
        {
            pdp = (PageTable*)((uint64_t)pageEntry.GetAddress() << 12);
        }

        //Getting page directory
        pageEntry = pdp->entries[pageDirectory];
        PageTable* pd;
        if (!pageEntry.GetFlag(PageEntryFlags::Present))
        {
            pd = (PageTable*)PageFrameAllocator::The()->RequestPage();
            sl::memset(pd, 0, PAGE_SIZE);

            pageEntry.SetAddress((uint64_t)pd >> 12);
            pageEntry.SetFlag(PageEntryFlags::Present, true);
            pageEntry.SetFlag(PageEntryFlags::ReadWrite, true);
            pdp->entries[pageDirectory] = pageEntry;
        }
        else
        {
            pd = (PageTable*)((uint64_t)pageEntry.GetAddress() << 12);
        }

        //Getting page table
        pageEntry = pd->entries[pageTable];
        PageTable* pt;
        if (!pageEntry.GetFlag(PageEntryFlags::Present))
        {
            pt = (PageTable*)PageFrameAllocator::The()->RequestPage();
            sl::memset(pt, 0, PAGE_SIZE);

            pageEntry.SetAddress((uint64_t)pt >> 12);
            pageEntry.SetFlag(PageEntryFlags::Present, true);
            pageEntry.SetFlag(PageEntryFlags::ReadWrite, true);
            pd->entries[pageTable] = pageEntry;
        }
        else
        {
            pt = (PageTable*)((uint64_t)pageEntry.GetAddress() << 12);
        }

        //Getting the page entry itself
        pageEntry = pt->entries[page];
        pageEntry.SetAddress((uint64_t)physicalAddr >> 12);
        pageEntry.SetFlag(PageEntryFlags::Present, true);
        pageEntry.SetFlag(PageEntryFlags::ReadWrite, true);
        pt->entries[page] = pageEntry;
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
    }

    void PageTableManager::MakeCurrentMap()
    {
        Drivers::CPU::LoadPageTableMap(pml4Addr);
    }
}
