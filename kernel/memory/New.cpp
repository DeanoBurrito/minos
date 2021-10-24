#include <memory/KHeap.h>

namespace std
{
    //thanks gcc - really good definition there.
    enum class align_val_t : size_t {};
}

//TODO: implement the remaining global new/delete operators for the kernel

void* operator new(size_t size)
{
    return Kernel::KHeap::The()->Malloc(size);
}

void* operator new[](size_t size)
{
    return Kernel::KHeap::The()->Malloc(size);
}

void operator delete(void* addr) noexcept
{
    Kernel::KHeap::The()->Free(addr);
}

void operator delete(void* addr, unsigned long) noexcept
{
    //fuck you _ZdlPvm
    Kernel::KHeap::The()->Free(addr);
}

void operator delete[](void* addr) noexcept
{
    Kernel::KHeap::The()->Free(addr);
}

void operator delete[](void* addr, unsigned long) noexcept
{
    Kernel::KHeap::The()->Free(addr);
}

/*
    From my understanding these are used for c++ exceptions (which are disabled!).
    gxx_personality is for stack unwinding, because it uses a different method to dwarf2.
*/
void* __gxx_personality_v0 = 0;
void* _Unwind_Resume = 0;
