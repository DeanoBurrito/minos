#include "PageTableManager.h"
#include "PageFrameAllocator.h"
#include "Memory.h"
#include "CPU.h"

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

    PageTableManager::PageTableManager(PageTable* pml4Address)
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
            memset(pdp, 0, PAGE_SIZE);

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
            memset(pd, 0, PAGE_SIZE);

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
            memset(pt, 0, PAGE_SIZE);

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

    void PageTableManager::MakeCurrentMap()
    {
        CPU::LoadPageTableMap(pml4Addr);
    }
}
