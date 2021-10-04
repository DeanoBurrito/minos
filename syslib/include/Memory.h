#pragma once

#include <stddef.h>
#include <stdint.h>

namespace sl
{
    /*
        Compiles away to a few instructions here and there. Makes pointer maths really easy, especially when
        switching between integers and real pointers.
    */
    template <typename BackingType>
    union IntPtr
    {
        BackingType raw;
        void* ptr;

        IntPtr() : raw(0) {}
        IntPtr(BackingType r) : raw(r) {}
        IntPtr(void* p) : ptr(p) {}

        //type safety is thrown to the wind here with reinterpret cast, but we've too cool for that.
        //NOTE: we are not.
        template <typename AsType>
        AsType* As()
        {
            return reinterpret_cast<AsType*>(ptr);
        }
    };

    //99% of the time this is what we want, just saving some typing in the future.
    typedef IntPtr<uint64_t> UIntPtr;

    template <typename WordType>
    __attribute__((always_inline)) inline void MemWrite(sl::UIntPtr where, WordType val)
    {
        *reinterpret_cast<volatile WordType*>(where.ptr) = val;
    }

    template <typename WordType>
    __attribute__((always_inline)) inline WordType MemRead(sl::UIntPtr where)
    {
        return *reinterpret_cast<volatile WordType*>(where.ptr);
    }

    //Allocates a number of bytes within the current stack frame.
    __attribute__((always_inline)) inline void* StackAlloc(size_t size)
    {
        return __builtin_alloca(size);
    }
    
    //std::move replacement
    template <typename T>
    T&& move(T&& t)
    {
        return static_cast<T&&>(t);
    }

    //std::swap replacement
    template <typename T>
    void swap(T& a, T& b)
    {
        T temp = move(a);
        a = move(b);
        b = move(temp);
    }
    
    //sets count bytes to value from start.
    void memset(void* const start, uint8_t value, size_t count);

    //copies count bytes from source to dest.
    void memcopy(const void* const source, void* const destination, size_t count);
    //copies count bytes from source to dest, with copy treating offsets as 'zero'. Operation is bounds checked.
    void memcopy(const void* const source, size_t sourceOffset, void* const dest, size_t destOffset, size_t count);

    //compares count bytes from a and b. Returns 0 if equal for count.
    int memcmp(const void* const a, const void* const b, size_t count);

    //returns the offset of the first instance of target from buff. UpperLimit is a max number of bytes to scan, 0 means no limit. 0 means target was not found.
    size_t memfirst(const void* const buff, uint8_t target, size_t upperLimit);
    //returns the offset of the first instance of target from (buff + offset). Upperlimit is max bytes to scan, 0 meaning no limit. Return of 0 means target was not found.
    size_t memfirst(const void* const buff, size_t offset, uint8_t target, size_t upperLimit);
}
