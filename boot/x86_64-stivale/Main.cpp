#include "Stivale2.h"
#include "../BootInfo.h"

extern "C" void StivaleMain()
{
    asm volatile("mov $0xdeadc0de, %rax");
    while (1);
}
