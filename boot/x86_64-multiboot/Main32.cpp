#include "Multiboot.h"

extern "C"
__attribute__((cdecl)) void MultibootMain(uint32_t magic, uint32_t infoAddr)
{
    //still in protected mode, need to enter long mode, populated bootinfo, just into kernelmain.
    while (1);
}