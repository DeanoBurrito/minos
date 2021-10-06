#include <stddef.h>
#include "Multiboot.h"
#include "Loader.h"
#include "../BootInfo.h"
#include "../X86Helpers.h"

extern "C" __attribute__((noreturn, sysv_abi)) void MultibootMain(uint32_t magic, uint32_t infoAddr)
{
    //NOTE: gdb dosnt seem to print local variables correctly here, might be a cross-platform thing.
    
    SerialInit();

    Log("Minos Multiboot-1 pre-kernel initialized, arguments below: ");
    Log(UIntToString(magic));
    Log(UIntToString(infoAddr));

    if (magic != MULTIBOOT_ECHO_MAGIC)
    {
        Log("Multiboot magic echo not present, init data may be meaningless. Aborting init.");
        error();
    }
    else
        Log("Multiboot magic valid.");
    
    MbInfo* multibootInfo = reinterpret_cast<MbInfo*>(infoAddr);
    //Parse mb tables and create our own bootInfo

    //setup barebones gdt/idt/paging structure, jump into long mode

    //Load Kernel
    if (!KernelValid())
    {
        Log("Kernel file was invalid?! Aborting load.");
        error();
    }
    else if (!LoadKernel())
    {
        Log("Failed to load kernel file. Aborting load.");
        error();
    }

    //find kernel entry and jump to it
    void (*KernelMain)(BootInfo*) = (__attribute__((sysv_abi)) void(*)(BootInfo*))GetKernelEntry();
    //KernelMain(bootInfo);

    Log("Kernel main has returned to pre-kernel!");
    __builtin_unreachable();
}