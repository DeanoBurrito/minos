#pragma once

#include <stddef.h>

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

    public:
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
                return nullptr; //TODO: emit an error
            return ptr;
        }

        void Free(const void* const ptr)
        {
            TryFree(ptr); //TODO: emit an error
        }
    };

#ifdef SL_ENABLE_TEST_ALLOCATORS
    //No real use, other than testing
    class FailureAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return false; }

        bool TryFree(void*)
        { return false; }
    };

    //No real use, other than testing
    class SuccessAllocator
    {
    public:
        bool TryAlloc(void**, const size_t)
        { return true; }

        bool TryFree(void*)
        { return true; }
    };
#endif
}
