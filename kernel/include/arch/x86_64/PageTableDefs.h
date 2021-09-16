#pragma once

#include <Platform.h>
#include <stdint.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

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
    } __attribute__((aligned(PAGE_SIZE)));
}
