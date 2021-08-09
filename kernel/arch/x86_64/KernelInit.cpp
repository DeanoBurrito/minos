#include <Interrupts.h>
#include <IDT.h>

namespace Kernel
{
    void InitPlatformInterrupts(IDTR* idtr)
    {
        idtr->SetEntry((void*)InterruptHandlers::DoubleFault, INTERRUPT_VECTOR_DOUBLE_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr->SetEntry((void*)InterruptHandlers::GeneralProtectionFault, INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT, IDT_ATTRIBS_InterruptGate, 0x08);
        idtr->SetEntry((void*)InterruptHandlers::PageFault, INTERRUPT_VECTOR_PAGE_FAULT, IDT_ATTRIBS_InterruptGate, 0x8);
    }
}