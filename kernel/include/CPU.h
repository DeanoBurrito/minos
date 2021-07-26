#pragma once

#include "GDT.h"
#include "IDT.h"

#define PORT_USUALLY_EMPTY 0x80

namespace Kernel
{
    class CPU
    {
    public:
        static void Init();
 
        static void EnableInterrupts();
        static void DisableInterrupts();

        static void LoadPageTableMap(void* toplevelAddress);
        static void LoadGDT(GDTDescriptor* address);
        static void LoadIDT(IDTR* idtr);
        static void Halt();

        static void PortWrite8(uint16_t port, uint8_t data);
        static void PortWrite16(uint16_t port, uint16_t data);
        static void PortWrite32(uint16_t port, uint32_t data);

        static uint8_t PortRead8(uint16_t port);
        static uint16_t PortRead16(uint16_t port);
        static uint32_t PortRead32(uint16_t port);

        static void PortIOWait(); //waits until IO bus has completed any outstanding operations.

        static char* GetArchitectureName();
    };
}
