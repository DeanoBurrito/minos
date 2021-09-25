#include <stddef.h>
#include "Multiboot.h"
#include "Loader.h"
#include "../BootInfo.h"

//These are here incase we want to move away from gcc builtins later
#define memset(source, val, count) __builtin_memset(source, val, count)

#define SERIAL_PORT 0x03F8

extern "C" void error();

inline __attribute__((always_inline)) void OutB(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

inline __attribute__((always_inline)) uint8_t InB(uint16_t port)
{
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

char getIntBuffer[64];
char* UIntToString(uint32_t value)
{
    memset(getIntBuffer, 0, 64);

    size_t index = 0;
    while (value)
    {
        uint32_t remainder = value % 16;
        value = value / 16;
        if (remainder < 10)
            getIntBuffer[index] = '0' + remainder;
        else
            getIntBuffer[index] = 'A' + remainder - 10;
        index++;
    }

    if (index == 0 && value == 0)
    {
        getIntBuffer[index] = '0';
        index++;
    }
    getIntBuffer[index] = 'x';
    getIntBuffer[index + 1] = '0';
    index += 2;

    char temp;
    for (size_t i = 0; i < index / 2; i++)
    {
        temp = getIntBuffer[i];
        getIntBuffer[i] = getIntBuffer[index - i - 1];
        getIntBuffer[index - i - 1] = temp;
    }

    return getIntBuffer;
}

void Log(const char* msg)
{
    for (size_t i = 0; msg[i] != 0; i++)
    {
        //once ready to send is set, write 1 char at a time to port
        uint8_t serialFlags = 0;
        uint16_t flagsPort = SERIAL_PORT + 5;
        while ((serialFlags & (1 << 5)) != 0)
            serialFlags = InB(flagsPort);

        OutB(SERIAL_PORT, msg[i]);
    }

    //newline
    OutB(SERIAL_PORT, '\r');
    OutB(SERIAL_PORT, '\n');
}

extern "C" __attribute__((noreturn, sysv_abi)) void MultibootMain(uint32_t magic, uint32_t infoAddr)
{
    //NOTE: gdb dosnt seem to print local variables correctly here, might be a cross-platform thing.
    
    //cheeky serial init, so we can output to qemu
    OutB(SERIAL_PORT + 1, 0x00); //disable interrupts
    OutB(SERIAL_PORT + 3, 0x80); //enabled DLAB
    OutB(SERIAL_PORT + 0, 0x03); //set divisor 3
    OutB(SERIAL_PORT + 1, 0x00); //high byte
    OutB(SERIAL_PORT + 3, 0x03); //usual stop and parity settings
    OutB(SERIAL_PORT + 2, 0xC7); //fifio, cleared with usual settings
    OutB(SERIAL_PORT + 4, 0x0F); //skipping test and enabling settings

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