#pragma once

//https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
#include <stdint.h>

struct MbInfo
{
    union flags
    {
        uint32_t raw;
        struct
        {
            uint8_t memorySizeAvailable : 1;
            uint8_t bootDeviceAvailable : 1;
            uint8_t commandLineAvailable : 1;
            uint8_t modsAvailable : 1;
            uint8_t reserved0 : 1; 
            uint8_t reserved1 : 1;
            uint8_t memoryMapAvailable : 1;
            uint8_t reserved2 : 1;
            uint8_t configTableAvailable : 1;
            uint8_t bootloaderNameAvailable : 1;
            uint8_t reserved3 : 1; //normally APM, but we're using the chad ACPI
            uint8_t vbeAvailable : 1; //only declares vbe1 and vbe2 interfaces, vbe3+ must be discovered ourselves.
            uint8_t framebufferAvailable : 1;
        };
    } __attribute__((packed)) flags;

    //valid if flags[0]: lower memory available in kilobytes
    uint32_t memoryLower;
    //valid if flags[0]: higher memory (>1MB) available in kilobytes
    uint32_t memoryHigher;

    //valid if flags[1]: bios disk device os image was loaded from
    uint32_t bootDevice;

    //valid if flags[2]: pointer to cstring passed as startup args
    uint32_t cmdline;

    //valid if flags[3]: number of modules and where they were loaded
    uint32_t modsCount;
    uint32_t modsAddr;

    //valid if flags[4] OR flags[5]: stuff for debuging kernel images, ignored.
    uint32_t syms[4];

    //valid if flags[6]: contains a pointer to memory map as provided by bios/uefi
    uint32_t mmapLength;
    uint32_t mmapAddr;

    //valid if flags[7]: bios drive info
    uint32_t drivesLength;
    uint32_t drivesAddr;

    //valid if flags[8]: returns BIOS config stuff. yikes.
    uint32_t configTable;

    //valid if flags[9]: pointer to cstring of bootloader name
    uint32_t bootLoaderName;

    //valid if flags[10]: old school bios power management stuff
    uint32_t apmTable;

    //valid if flags[11]: details for interacting with BIOS vbe calls
    uint32_t vbeControlInfo;
    uint32_t vbeModeInfo;
    uint16_t vbeMode;
    uint16_t vbeInterfaceSeg;
    uint16_t vbeInterfaceOff;
    uint16_t vbeInterfaceLen;

    //valid if flags[12]: 
    uint64_t framebufferAddr;
    uint32_t framebufferPitch;
    uint32_t framebufferWidth;
    uint32_t framebufferHeight;
    uint8_t framebufferBitsPerPixel;
    uint8_t framebufferType;
    uint8_t colourInfo[6];
} __attribute__((packed));

#define MB_MEMORY_TYPE_AVAILABLE 1
#define MB_MEMORY_TYPE_ACPI_RECLAIMABLE 3
#define MB_FRAMEBUFFER_TYPE_INDEXED 0
#define MB_FRAMEBUFFER_TYPE_DIRECT 1

#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_ECHO_MAGIC 0x2BADB002

//NOTE: structure needs to be offset by -4 bytes when creating from pointers.
struct MbMemoryMap
{
    uint32_t size;
    uint64_t baseAddress; 
    uint64_t length;
    uint32_t type;
} __attribute__((packed));
