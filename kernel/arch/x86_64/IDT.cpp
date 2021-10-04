#include <arch/x86_64/IDT.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

namespace Kernel
{
    void IDTEntry::SetOffset(uint64_t offset)
    {
        offset0 = (uint16_t)((offset & 0x000000000000FFFF) >> 0);
        offset1 = (uint16_t)((offset & 0x00000000FFFF0000) >> 16);
        offset2 = (uint32_t)((offset & 0xFFFFFFFF00000000) >> 32);
    }
}
