#include <stddef.h>
#include <stdint.h>
#include "Stivale2.h"
#include "../BootInfo.h"
#include "../X86Helpers.h"

extern "C" void StivaleMain(stivale2_struct* stivale)
{
    SerialInit();

    Log("Hello stivaleeeee!");
    
    asm volatile("mov $0xdeadc0de, %rax");
    while (1);
}
