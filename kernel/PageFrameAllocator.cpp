#include <StringExtras.h>
#include <PageFrameAllocator.h>
#include <Platform.h>

namespace Kernel
{
    PageFrameAllocator globalPageManager;
    PageFrameAllocator* PageFrameAllocator::The()
    {
        return &globalPageManager;
    }

    void PageFrameAllocator::InitBitmap(uint64_t size, void* bufferAddr)
    {
        pageBitmap.size = size;
        pageBitmap.buffer = (uint8_t*)bufferAddr;
        for (size_t i = 0; i < size; i++)
        {
            *(uint8_t*)(pageBitmap.buffer + i) = 0;
        }
    }

    void PageFrameAllocator::ReservePage(void* addr)
    {
        uint64_t index = (uint64_t)addr / PAGE_SIZE;
        if (pageBitmap[index])
            return;

        if (!pageBitmap.Set(index, true))
            return;

        reservedMemory += PAGE_SIZE;
        freeMemory -= PAGE_SIZE;
    }

    void PageFrameAllocator::ReservePages(void* addr, uint64_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            ReservePage((void*)((uint64_t)addr + (i * PAGE_SIZE)));
        }
    }

    void PageFrameAllocator::Init(BootInfo* bootInfo)
    {
        rootDescriptor = bootInfo->memoryDescriptors;
        descriptorCount = bootInfo->memoryDescriptorsCount;
        
        //find largest segment of usuable memory
        MemoryRegionDescriptor* largest = nullptr;
        MemoryRegionDescriptor* scan = rootDescriptor;
        uint64_t largestSize = 0;
        for (size_t i = 0; i < descriptorCount; i++)
        {
            if (scan->flags == MEMORY_DESCRIPTOR_FREE)
            {
                if (scan->numberOfPages > largestSize)
                {
                    largestSize = scan->numberOfPages;
                    largest = scan;
                }
            }
            scan++;
        }

        //initialize bitmap
        uint64_t memorySize = GetTotalMemory();
        freeMemory = memorySize;
        uint64_t bitmapSize = memorySize / PAGE_SIZE / 8 + 1;

        InitBitmap(bitmapSize, (void*)largest->physicalStart);

        //lock bitmap pages, and set bits for reserved memory
        LockPages(&pageBitmap, pageBitmap.size / PAGE_SIZE + 1);
        scan = rootDescriptor;
        for (size_t i = 0; i < descriptorCount; i++)
        {
            if (scan->flags == MEMORY_DESCRIPTOR_RESERVED)
                ReservePages((void*)scan->physicalStart, scan->numberOfPages);
            scan++;
        }
    }

    void PageFrameAllocator::FreePage(void* addr)
    {
        uint64_t index = (uint64_t)addr / PAGE_SIZE;
        if (!pageBitmap[index])
            return; //TODO: check if page is reserved. If it is, ignore the call to free the page.

        if (!pageBitmap.Set(index, false))
            return; //dud address, just return.

        freeMemory += PAGE_SIZE;
        usedMemory -= PAGE_SIZE;
        if (index < allocateStartIndex)
            allocateStartIndex = index;
    }

    void PageFrameAllocator::FreePages(void* addr, uint64_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            FreePage((void*)((uint64_t)addr + (i * PAGE_SIZE)));
        }
    }

    void PageFrameAllocator::LockPage(void* addr)
    {
        uint64_t index = (uint64_t)addr / PAGE_SIZE;
        if (pageBitmap[index])
            return;

        if (!pageBitmap.Set(index, true))
            return;

        usedMemory += PAGE_SIZE;
        freeMemory -= PAGE_SIZE;
    }

    void PageFrameAllocator::LockPages(void* addr, uint64_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            LockPage((void*)((uint64_t)addr + (i * PAGE_SIZE)));
        }
    }

    void *PageFrameAllocator::RequestPage()
    {
        for (uint64_t index = allocateStartIndex; index < pageBitmap.size * 8; index++)
        {
            if (pageBitmap[index])
                continue;

            LockPage((void*)(index * PAGE_SIZE));
            allocateStartIndex = index + 1;
            return (void*)(index * PAGE_SIZE);
        }

        //BOOM! Out of free pages, display error message here, and eventually implement swapping (we'd swap out a page here, and return a new section of memory)
        return nullptr;
    }

    Memory::MemoryUsage PageFrameAllocator::GetMemoryUsage()
    {
        Memory::MemoryUsage usage(GetTotalMemory(), freeMemory, reservedMemory, usedMemory);
        return usage;
    }

    uint64_t PageFrameAllocator::GetTotalMemory()
    {
        static uint64_t memoryBytes = 0;
        if (memoryBytes > 0)
            return memoryBytes;

        MemoryRegionDescriptor* scan = rootDescriptor;
        for (size_t i = 0; i < descriptorCount; i++)
        {
            memoryBytes += scan->numberOfPages * PAGE_SIZE;
            scan++;
        }

        return memoryBytes;
    }
}
