#include "Multiboot.h"
#include <stddef.h>

#define SERIAL_PORT 0x03F8

inline __attribute__((always_inline)) void OutB(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

void Log(const char* msg)
{
    for (size_t i = 0; msg[i] != 0; i++)
    {
        //once ready to send is set, write 1 char at a time to port
        uint8_t serialFlags = 0;
        uint16_t flagsPort = SERIAL_PORT + 5;
        while ((serialFlags & (1 << 5)) != 0)
            asm volatile("inb %1, %0" : "=a"(serialFlags) : "Nd"(flagsPort));

        OutB(SERIAL_PORT, msg[i]);
    }

    //newline
    OutB(SERIAL_PORT, '\r');
    OutB(SERIAL_PORT, '\n');
}

extern "C"
__attribute__((cdecl)) void MultibootMain(uint32_t magic, uint32_t infoAddr)
{
    //cheeky serial init, so we can output to qemu
    OutB(SERIAL_PORT + 1, 0x00); //disable interrupts
    OutB(SERIAL_PORT + 3, 0x80); //enabled DLAB
    OutB(SERIAL_PORT + 0, 0x03); //set divisor 3
    OutB(SERIAL_PORT + 1, 0x00); //high byte
    OutB(SERIAL_PORT + 3, 0x03); //usual stop and parity settings
    OutB(SERIAL_PORT + 2, 0xC7); //fifio, cleared with usual settings
    OutB(SERIAL_PORT + 4, 0x0F); //skipping test and enabling settings

    Log("Hello world!");

    while (1);
}