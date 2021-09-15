#pragma once

#include <stdint.h>
#include <BootInfo.h>
#include <Bitmap.h>
#include <memory/MemoryUsage.h>

#define PAGE_SIZE 4096

namespace Kernel
{
    class PageFrameAllocator
    {
    private:
        MemoryRegionDescriptor* rootDescriptor;
        uint64_t descriptorCount;

        Bitmap pageBitmap;
        uint64_t freeMemory;
        uint64_t reservedMemory;
        uint64_t usedMemory;

        uint64_t allocateStartIndex;

        void InitBitmap(uint64_t size, void* bufferAddr);

    public:
        static PageFrameAllocator* The();

        void Init(BootInfo* bootInfo);

        void ReservePage(void* addr);
        void ReservePages(void* addr, uint64_t count);
        void FreePage(void* addr);
        void FreePages(void* addr, uint64_t count);
        void LockPage(void* addr);
        void LockPages(void* addr, uint64_t count);

        void* RequestPage();

        Memory::MemoryUsage GetMemoryUsage();
        uint64_t GetTotalMemory();
    };
}
