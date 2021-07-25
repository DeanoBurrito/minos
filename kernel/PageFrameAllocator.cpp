#include "StringUtil.h"
#include "EfiDefs.h"
#include "KRenderer.h"
#include "PageFrameAllocator.h"

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
        for (int i = 0; i < size; i++)
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
        for (int i = 0; i < count; i++)
        {
            ReservePage((void*)((uint64_t)addr + (i * PAGE_SIZE)));
        }
    }

    void PageFrameAllocator::UnreservePage(void* addr)
    {
        uint64_t index = (uint64_t)addr / PAGE_SIZE;
        if (!pageBitmap[index])
            return;

        if (!pageBitmap.Set(index, false))
            return;

        freeMemory += PAGE_SIZE;
        reservedMemory -= PAGE_SIZE;
        if (index < allocateStartIndex)
            allocateStartIndex = index;
    }

    void PageFrameAllocator::UnreservePages(void* addr, uint64_t count)
    {
        for (int i = 0; i < count; i++)
        {
            UnreservePage((void*)((uint64_t)addr + (i * PAGE_SIZE)));
        }
    }

    void PageFrameAllocator::Init(BootInfo* bootInfo)
    {
        descriptorSize = bootInfo->memoryMap.descriptorSize;
        descriptorCount = bootInfo->memoryMap.size / bootInfo->memoryMap.descriptorSize;
        descriptorMapSize = bootInfo->memoryMap.size;
        rootDescriptor = bootInfo->memoryMap.descriptor;

        //find largest segment of usable memory
        EfiMemoryDescriptor* largestDescriptor = nullptr;
        uint64_t largestSize = 0;
        for (int i = 0; i < descriptorCount; i++)
        {
            EfiMemoryDescriptor* localDesc = (EfiMemoryDescriptor*)((uint64_t)rootDescriptor + (i * descriptorSize));
            if (localDesc->type == (uint32_t)EfiMemoryDescriptorType::EfiConventionalMemory)
            {
                if (localDesc->numberOfPages * PAGE_SIZE > largestSize)
                {
                    largestSize = localDesc->numberOfPages * PAGE_SIZE;
                    largestDescriptor = localDesc;
                }
            }
        }

        //initialize bitmap
        uint64_t memorySize = GetTotalMemory();
        freeMemory = memorySize;
        uint64_t bitmapSize = memorySize / PAGE_SIZE / 8 + 1;

        InitBitmap(bitmapSize, (void*)largestDescriptor->physicalStart);

        //lock bitmap pages, and set bits for reserved memory
        LockPages(&pageBitmap, pageBitmap.size / PAGE_SIZE + 1);
        for (int i = 0; i < descriptorCount; i++)
        {
            EfiMemoryDescriptor* localDesc = (EfiMemoryDescriptor*)((uint64_t)rootDescriptor + (i * descriptorSize));
            if (localDesc->type != (uint32_t)EfiMemoryDescriptorType::EfiConventionalMemory)
            {
                //not conventional memory, reserve it
                ReservePages((void*)localDesc->physicalStart, localDesc->numberOfPages);
            }
        }
    }

    void PageFrameAllocator::GetPageMapIndices(uint64_t virtualAddress, uint64_t* pdpIndex, uint64_t* pdIndex, uint64_t* ptIndex, uint64_t* pageIndex)
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

    void PageFrameAllocator::FreePage(void* addr)
    {
        uint64_t index = (uint64_t)addr / PAGE_SIZE;
        if (!pageBitmap[index])
            return;

        if (!pageBitmap.Set(index, false))
            return; //TODO: throw an error here

        freeMemory += PAGE_SIZE;
        usedMemory -= PAGE_SIZE;
        if (index < allocateStartIndex)
            allocateStartIndex = index;
    }

    void PageFrameAllocator::FreePages(void* addr, uint64_t count)
    {
        for (int i = 0; i < count; i++)
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
        for (int i = 0; i < count; i++)
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

    MemoryUsage PageFrameAllocator::GetMemoryUsage()
    {
        MemoryUsage usage(GetTotalMemory(), freeMemory, reservedMemory, usedMemory);
        return usage;
    }

    uint64_t PageFrameAllocator::GetTotalMemory()
    {
        static uint64_t memoryBytes = 0;
        if (memoryBytes > 0)
            return memoryBytes;

        for (uint64_t i = 0; i < descriptorCount; i++)
        {
            EfiMemoryDescriptor* descriptor = (EfiMemoryDescriptor*)((uint64_t)rootDescriptor + (i * descriptorSize));
            memoryBytes += descriptor->numberOfPages * PAGE_SIZE;
        }

        return memoryBytes;
    }

    void PageFrameAllocator::DumpMemoryInfo()
    {
        for (int i = 0; i < descriptorCount; i++)
        {
            EfiMemoryDescriptor* descriptor = (EfiMemoryDescriptor*)((uint64_t)rootDescriptor + (i * descriptorSize));
            KRenderer::The()->Write(ToStr((uint64_t)i));
            if (i < 10)
                KRenderer::The()->Write("   ");
            else if (i < 100)
                KRenderer::The()->Write("  ");
            else
                KRenderer::The()->Write(" ");
            KRenderer::The()->Write(EFI_MEMORY_TYPE_STRINGS[descriptor->type]);

            KRenderer::The()->Write("    size=");
            KRenderer::The()->Write(ToStr(descriptor->numberOfPages * PAGE_SIZE / 1024));
            KRenderer::The()->Write("KB");
            KRenderer::The()->Write(", addr=");
            KRenderer::The()->WriteLine(ToStrHex(descriptor->physicalStart));
        }
        KRenderer::The()->Write("Entries count: ");
        KRenderer::The()->WriteLine(ToStr(descriptorCount));

        KRenderer::The()->Write("Total mapped memory: ");
        KRenderer::The()->WriteLine(ToStr(GetTotalMemory()));
    }
}
