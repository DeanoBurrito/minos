#include <arch/x86_64/IDT.h>

namespace Kernel
{
    void IDTDescriptorEntry::SetOffset(uint64_t offset)
    {
        offset0 = (uint16_t)((offset & 0x000000000000FFFF) >> 0);
        offset1 = (uint16_t)((offset & 0x00000000FFFF0000) >> 16);
        offset2 = (uint32_t)((offset & 0xFFFFFFFF00000000) >> 32);
    }

    uint64_t IDTDescriptorEntry::GetOffset()
    {
        uint64_t offset = 0;

        offset |= (uint64_t)offset0;
        offset |= ((uint64_t)offset1 << 16);
        offset |= ((uint64_t)offset2 << 32);

        return offset;
    }

    void IDTR::SetEntry(void* handler, uint64_t vector, uint8_t typesAttribs, uint16_t selector)
    {
        IDTDescriptorEntry* entry = (IDTDescriptorEntry*)(offset + vector * sizeof(IDTDescriptorEntry));
        entry->SetOffset((uint64_t)handler);
        entry->attributes = typesAttribs;
        entry->selector = selector;
    }
}
