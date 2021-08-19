#include <PageTableManager.h>
#include <PageFrameAllocator.h> 
#include <memory/KHeap.h>

//malloc will round up to the nearest 'x' bytes. 0x10 = 16 bytes
#define MALLOC_CHUNK_SIZE 0x10

namespace Kernel
{
    void HeapSegmentHeader::CombineForward(HeapSegmentHeader* lastSegment)
    {
        if (next == nullptr)
            return;
        if (!next->free)
            return;
        
        if (next == lastSegment)
            lastSegment = this;
        
        length = length + next->length + sizeof(HeapSegmentHeader);

        if (next->next != nullptr)
            next->next->prev = this;
        next = next->next;
    }

    void HeapSegmentHeader::CombineBackward(HeapSegmentHeader* lastSegment)
    {
        if (prev != nullptr && prev->free)
            prev->CombineForward(lastSegment);
    }

    HeapSegmentHeader* HeapSegmentHeader::Split(size_t biteSize, HeapSegmentHeader* lastSegment)
    { 
        if (biteSize < MALLOC_CHUNK_SIZE)
            return nullptr;
        
        int64_t splitLength = length - biteSize - sizeof(HeapSegmentHeader);
        if (splitLength < MALLOC_CHUNK_SIZE)
            return nullptr;

        HeapSegmentHeader* newHeader = (HeapSegmentHeader*)((size_t)this + biteSize + sizeof(HeapSegmentHeader));
        //classic double linked list insertion
        if (next != nullptr)
            next->prev = newHeader;
        newHeader->next = next;
        next = newHeader;
        newHeader->prev = this;

        //setting lengths of both headers to correct values, and copying free to new header
        newHeader->length = splitLength;
        length = biteSize;
        newHeader->free = free;

        if (lastSegment == this)
            lastSegment = newHeader;

        return newHeader;
    }
    
    KHeap instance;
    KHeap* KHeap::The()
    {
        return &instance;
    }

    void KHeap::Init(void* address, size_t initialLength)
    {
        initialLength = (initialLength + PAGE_SIZE - 1) / PAGE_SIZE;

        size_t position = (size_t)address;

        //map some initial pages so we dont instantly page fault
        for (size_t i = 0; i < initialLength; i++)
        {
            PageTableManager::The()->MapMemory((void*)position, PageFrameAllocator::The()->RequestPage());
            position += PAGE_SIZE;
        }

        //keep track of those variables
        size_t heapLength = initialLength * PAGE_SIZE;
        heapStart = address;
        heapEnd = (void*)((size_t)heapStart + heapLength);

        //create initial heap segment
        HeapSegmentHeader* firstSegment = (HeapSegmentHeader*)address;
        firstSegment->length = heapLength - sizeof(HeapSegmentHeader); //dont forget that these headers take up space
        firstSegment->next = nullptr;
        firstSegment->free = true;
        firstSegment->prev = nullptr;

        lastSegment = firstSegment;
    }

    void* KHeap::Malloc(size_t size)
    { 
        if (size % MALLOC_CHUNK_SIZE > 0)
        {
            //round up to nearest chunk size
            size -= size % MALLOC_CHUNK_SIZE;
            size += MALLOC_CHUNK_SIZE;
        }

        if (size == 0)
            return nullptr;

        //search through linked list and until we find one big enough
        HeapSegmentHeader* scanHead = (HeapSegmentHeader*)heapStart;
        while (true)
        {
            if (scanHead->free)
            {
                if (scanHead->length > size)
                {
                    //carve out the space we need with split(), then return the address of the free space after this segment.
                    scanHead->Split(size, lastSegment);
                    scanHead->free = false;
                    return (void*)((uint64_t)scanHead + sizeof(HeapSegmentHeader));
                }
                if (scanHead->length == size) //perfect fit, no need to adjust segment sizes
                {
                    scanHead->free = false;
                    return (void*)((uint64_t)scanHead + sizeof(HeapSegmentHeader));
                }
            }

            if (scanHead->next == nullptr)
                break;
            
            scanHead = scanHead->next;
        }

        //hamdle error case of no free space on heap
        ExpandHeap(size);
        return Malloc(size); //guaranteed to only recurse once. Might even be able to use split() here and return earlier.
    }

    void KHeap::Free(void* address)
    {
        HeapSegmentHeader* header = (HeapSegmentHeader*)address - 1; //pointer maths, -1 here will decrement in steps of sizeof(HeapSegmentHeader);
        header->free = true;
        header->CombineForward(lastSegment);
        header->CombineBackward(lastSegment);
    }

    void KHeap::ExpandHeap(size_t expansionLength)
    {
        //round up to nearest page
        if (expansionLength % PAGE_SIZE > 0)
        {
            expansionLength -= expansionLength % PAGE_SIZE;
            expansionLength += PAGE_SIZE;
        }

        size_t requiredPages = expansionLength / PAGE_SIZE;
        HeapSegmentHeader* newSegment = (HeapSegmentHeader*)heapEnd; //stash current end

        for (size_t i = 0; i < requiredPages; i++)
        {
            PageTableManager::The()->MapMemory(heapEnd, PageFrameAllocator::The()->RequestPage());
            heapEnd = (void*)((size_t)heapEnd + PAGE_SIZE);
        }

        newSegment->free = true;
        newSegment->prev = lastSegment;
        lastSegment->next = newSegment;
        lastSegment = newSegment;
        newSegment->next = nullptr;

        newSegment->length = expansionLength - sizeof(HeapSegmentHeader);
        newSegment->CombineBackward(lastSegment);
    }

    size_t KHeap::GetCurrentUsage()
    {
        size_t totalUsage = 0;
        HeapSegmentHeader* current = (HeapSegmentHeader*)heapStart;
        while (current != nullptr)
        {
            if (!current->free)
                totalUsage += current->length;
            
            current = current->next;
        }

        return totalUsage;
    }
}