#pragma once

#include <stdint-gcc.h>
#include <BootInfo.h>
#include <Bitmap.h>

#define PAGE_SIZE 4096

namespace Kernel
{

    struct MemoryUsage
    {
        MemoryUsage(const uint64_t nTotal, const uint64_t nFree, const uint64_t nReserved, const uint64_t nUsed)
            : total{nTotal}, free{nFree}, reserved{nReserved}, used{nUsed}
        {
        }

        const uint64_t total;
        const uint64_t free;
        const uint64_t reserved;
        const uint64_t used;
    };

    class PageFrameAllocator
    {
    private:
        EfiMemoryDescriptor* rootDescriptor;
        uint64_t descriptorCount;
        uint64_t descriptorSize;
        uint64_t descriptorMapSize;

        Bitmap pageBitmap;
        uint64_t freeMemory;
        uint64_t reservedMemory;
        uint64_t usedMemory;

        uint64_t allocateStartIndex;

        void InitBitmap(uint64_t size, void* bufferAddr);

    public:
        static PageFrameAllocator* The();
        static void GetPageMapIndices(uint64_t virtualAddress, uint64_t* pdpIndex, uint64_t* pdIndex, uint64_t* ptIndex, uint64_t* pageIndex);

        void Init(BootInfo* bootInfo);

        void ReservePage(void* addr);
        void ReservePages(void* addr, uint64_t count);
        void FreePage(void* addr);
        void FreePages(void* addr, uint64_t count);
        void LockPage(void* addr);
        void LockPages(void* addr, uint64_t count);

        void* RequestPage();

        MemoryUsage GetMemoryUsage();
        uint64_t GetTotalMemory();
    };
}
