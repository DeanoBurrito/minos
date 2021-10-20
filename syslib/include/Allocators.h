#pragma once

#include <stddef.h>
#include <StringExtras.h>
#include <StringBuilder.h>

//not actually required, purely for convinience. 
#include <SlabAllocator.h>
#include <HeapAllocator.h>

/*
    This is inspired by a talk by Andrei Alexandrescu, in he talks about chaining multiple allocators
    together to make the most of their feature set. This is what CompositeALlocator is meant ot achieve:
    You can have an abstract memory alloc interface, with various layers of 'failover' if the previous one cannot
    provide the memory requested. For example: on a 64bit platform, you'll need lots of groups of 8 bytes
    (integers, doubles, and quad word pointers), so by chaining a slab alloc for 8 bytes as the primary, and
    a general heap as the secondary, you can massively speed up malloc/free of those sizes.
*/

//uncomment to enable always-fail and always-succeed allocators. Please note these dont actually manage any memory
//#define SL_ENABLE_TEST_ALLOCATORS

namespace sl
{
    //emitted when memory allocation fails.
    typedef void (*AllocError)(const char* msg);
    //emitted when freeing memory fails.
    typedef void (*FreeError)(const char* msg);
    //emitted for both alloc and free failures.
    typedef void (*GeneralAllocatorError)(const char* msg);

    struct AllocatorDebugSettings
    {   
        AllocError allocFailCallback = nullptr;
        FreeError freeFailCallback = nullptr;
        GeneralAllocatorError epicFailCallback = nullptr;
    };
    
    //The best allocator (also the reference implementation)
    class NullAllocator
    {
    public:
        //try a single allocation. If functin returns true, ptr is valid for size or greater, otherwise ptr contains it's previous value.
        bool TryAlloc(void** ptr, const size_t size)
        {
            *ptr = nullptr;
            return true;
        }

        //try to deallocate. Returns true if successfully able to free memory, false if this allocator is not responsible for this memory
        bool TryFree(const void* const ptr)
        {
            return ptr == nullptr;
        }
    };
    
    template <typename PrimaryAlloc, typename SecondaryAlloc>
    class CompositeAllocator
    {
    private:
        PrimaryAlloc primary;
        SecondaryAlloc secondary;

        AllocatorDebugSettings debugSettings;

    public:
        CompositeAllocator() = default;

        CompositeAllocator(AllocatorDebugSettings& dbgSettings)
        {
            debugSettings = sl::move(dbgSettings);
        }

        //clunky, but allows us to chain composite allocators
        bool TryAlloc(void** ptr, const size_t size)
        {
            if (primary.TryAlloc(ptr, size))
                return true;
            return secondary.TryAlloc(ptr, size);
        }

        bool TryFree(const void* const ptr)
        {
            if (primary.TryFree(ptr))
                return true;
        return secondary.TryFree(ptr);
        }

        void* Alloc(const size_t size)
        {
            void* ptr;
            if (!TryAlloc(&ptr, size))
            {
                StringBuilder bob;
                bob.Append("CompositeAllocator<T> failed to allocate 0x");
                bob.Append(sl::move(sl::UIntToString(size, BASE_HEX)));
                bob.Append(" bytes.");
                const string message = bob.ToString();

                if (debugSettings.allocFailCallback)
                    debugSettings.allocFailCallback(message.Data());
                if (debugSettings.epicFailCallback)
                    debugSettings.epicFailCallback(message.Data());
                return nullptr;
            }
            return ptr;
        }

        void Free(const void* const ptr)
        {
            if (!TryFree(ptr))
            {
                StringBuilder bob;
                bob.Append("CompositeAllocator<T> failed to free memory at address 0x");
                bob.Append(sl::move(sl::UIntToString((uint64_t)ptr, BASE_HEX)));
                const string message = bob.ToString();

                if (debugSettings.freeFailCallback)
                    debugSettings.freeFailCallback(message.Data());
                if (debugSettings.epicFailCallback)
                    debugSettings.epicFailCallback(message.Data());
            }
        }
    };

#ifdef SL_ENABLE_TEST_ALLOCATORS
    class FailureAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return false; }

        bool TryFree(void*)
        { return false; }
    };

    class SuccessAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return true; }

        bool TryFree(void*)
        { return true; }
    };

    class AllocOnlyAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return true; }

        bool TryFree(void*)
        { return false; }
    };

    class FreeOnlyAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return true; }

        bool TryFree(void*)
        { return false; }
    };
#endif
}
