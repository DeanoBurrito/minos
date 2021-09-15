#pragma once

#include <stdint.h>

namespace Kernel
{
    enum class PageEntryFlags : uint64_t
    {
        Present = 0,
        ReadWrite = 1,
        SuperOnly = 2,
        WriteThrough = 3,
        CacheDisabled = 4,
        Accessed = 5,
        Reserved0 = 6,
        LargePages = 7,
        Reserved1 = 8,

        ImplSpecific_0 = 9,
        ImplSpecific_1 = 10,
        ImplSpecific_2 = 11,

        NoExecute = 63,
    };

    enum class MemoryMapFlags : uint64_t
    {
        None = 0,

        //if not set, mapped memory is read only. ExecuteAllow being set will disable this.
        WriteAllow = 1 << 0,
        //if set, memory can be executed as code. Mutually exclusive with WriteAllow.
        ExecuteAllow = 1 << 1,
        //if set, will disable caching for that page.
        DisableCache = 1 << 2,
        //if set, user code can access this page, otherwise only kernel-mode code can access it.
        DisablePrivChecks = 1 << 3,
        //for mmapped devices/drivers. Driver has no intent to release/unmap this memory during os lifetime.
        EternalClaim = 1 << 4,
        //back this with physical memory now, rather than when first writen to.
        ImmediateAllocate = 1 << 5,
    };

    MemoryMapFlags operator&(MemoryMapFlags a, MemoryMapFlags b);
    MemoryMapFlags operator|(MemoryMapFlags a, MemoryMapFlags b);

    struct PageDirectoryEntry
    {
        uint64_t value;

        void SetFlag(PageEntryFlags flag, bool enabled);
        bool GetFlag(PageEntryFlags flag);
        void SetAddress(uint64_t address);
        uint64_t GetAddress();
    };

    struct PageTable
    {
        PageDirectoryEntry entries[512];
    } __attribute__((aligned(0x1000)));

    class PageTableManager
    {
    private:
        PageTable* pml4Addr;
        void* dummyPage; //readonly dummy page, lazily replaced with real pages as needed.
        bool noExecuteSupport;

        static void GetPageMapIndices(uint64_t virtualAddress, uint64_t* pdpIndex, uint64_t* pdIndex, uint64_t* ptIndex, uint64_t* pageIndex);

    public:
        static PageTableManager* The();

        void Init(PageTable* pml4Address);
        //map 1 page at the specified virtual address, with the requested flags.
        void MapMemory(void* virtualAddr, MemoryMapFlags flags);
        //maps a physical page to a virtual page, with requested flags. If physicalAddr is nullptr, it will be allocated.
        void MapMemory(void* virtualAddr, void* physicalAddr, MemoryMapFlags flags);
        //unmap page at the specified virtual address. This will free any backing memory, unless it was mapped with EternalClaim.
        void UnmapMemory(void* virtualAddr);

        void MakeCurrentMap();
    };
}
