#pragma once

#include <stddef.h>

namespace Kernel
{
    extern void* KMalloc(size_t);
    extern void KFree(void*);
}

namespace sl
{
    class HeapAllocator
    {
    public:
        bool TryAlloc(void** ptr, const size_t size)
        { 
            if (&Kernel::KMalloc != nullptr)
            {
                *ptr = Kernel::KMalloc(size);
                return true;
            }
            
            //TODO: userspace/general heap
            return false;
        }

        bool TryFree(const void* const ptr)
        { 
            if (&Kernel::KFree != nullptr)
            {
                Kernel::KFree(const_cast<void*>(ptr));
                return true;
            }

            return false;
        }

        void* Alloc(const size_t size)
        { 
            void* ptr = nullptr;
            TryAlloc(&ptr, size);
            return ptr;
        }

        void Free(const void* const ptr)
        {
            TryFree(ptr);
        }
    };
}
