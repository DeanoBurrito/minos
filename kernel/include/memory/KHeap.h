#pragma once

#include <stddef.h>

namespace Kernel
{
    class KHeap;
    
    class HeapSegmentHeader
    {
    friend KHeap;
    private:
        size_t length;
        HeapSegmentHeader* next;
        HeapSegmentHeader* prev;
        bool free;

        HeapSegmentHeader(size_t length) : length(length), next(nullptr), prev(nullptr), free(true) {}
        HeapSegmentHeader* Split(size_t biteSize, HeapSegmentHeader* lastSegment);
        void CombineForward(HeapSegmentHeader* lastSegment);
        void CombineBackward(HeapSegmentHeader* lastSegment);
    };
    
    class KHeap
    {
    private:
        void* heapStart;
        void* heapEnd;
        HeapSegmentHeader* lastSegment;

        void ExpandHeap(size_t expansionLength);

    public:
        static KHeap* The();

        void Init(void* address, size_t initialLength); 
        void* Malloc(size_t size);
        void Free(void* address);
        
        size_t GetCurrentUsage();
    };
}