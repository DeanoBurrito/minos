#pragma once

#include <stdint.h>

struct stivale2_header
{
    uint64_t entryPoint; //overrides elf entry point. 0 means ignore
    uint64_t stack; //32bit kernels must provide their own stack. 0 means limine will provide it (64bit only)
    
    union
    {
        uint64_t padding;
        struct
        {
            uint8_t reserved : 1; //previously used to set KASLR
            uint8_t higherHalf : 1; //if set will load in higher half memory
            uint8_t protectMemoryRanges : 1; //enables protected memory ranges (mapping -2GB block to elf headers, rather than all out fun)
        };
    } flags;

    uint64_t tags;
} __attribute__((packed));

struct stivale2_hdr_tag
{
    uint64_t id;
    uint64_t next; //0 for end of list, otherwise address of next tag
} __attribute__((packed));

#define STIVALE2_HEADER_TAG_ANY_VIDEO_ID 0xc75c9fa92a44c4db
struct stivale2_header_tag_any_video : public stivale2_hdr_tag
{
    uint64_t preference; //0=linear framebuffer, 1=dont care (CGA may be provided anyway)
} __attribute__((packed));

#define STIVALE2_HEADER_TAG_FRAMEBUFFER_ID 0x3ecc1bc43d0f7971
//can be used with or without any video. Without means we require a framebuffer.
struct stivale2_header_tag_framebuffer : public stivale2_hdr_tag
{
    uint16_t width; //if all values are 0, limine will select 'best fit' values
    uint16_t height;
    uint16_t bpp;
    uint16_t reserved;
} __attribute__((packed));

#define STIVALE2_HEADER_TAG_TERMINAL_ID 0xa85d499b1823be72
//allows kernel to interact with limine terminal
struct stivale2_header_tag_terminal : public stivale2_hdr_tag
{
    union flags
    {
        uint64_t padding;
        struct
        {
            uint8_t callbackPresent : 1;
        };
    };
    uint64_t callbackAddr;
} __attribute__((packed));

//no extra members so we just set the id and move on
#define STIVALE2_HEADER_TAG_5_LEVEL_PAGING_ID 0x932f477032007e8f

//tells the bootloader to unmap the first page, to cause PFs when accessing nullptr/NULL
#define STIVALE2_HEADER_TAG_UNMAP_NULL_PAGE_ID 0x92919432b16fe7e7

#define STIVALE2_HEADER_TAG_SMP_ID 0x1ab015085f3273df
struct stivale2_header_tag_smp : public stivale2_hdr_tag
{
    union flags
    {
        uint64_t padding;
        struct
        {
            uint8_t useBigApic : 1; //if set try to use X2APIC, otherwise just xAPIC
        };
    };
} __attribute__((packed));

struct stivale2_struct
{
    uint8_t brand[64];
    uint8_t version[64];

    uint64_t tags; //address of next tag
};

#define STIVALE2_STRUCT_TAG_PMRS_ID 0x5df266a64047b6bd
#define STIVALE2_PMR_EXECUTE_ALLOW 0x1
#define STIVALE2_PMR_WRITE_ALLOW 0x2
#define STIVALE2_PMR_READ_ALLOW 0x4
struct stivale2_pmr
{
    uint64_t base;
    uint64_t length;
    uint64_t permissions;
} __attribute__((packed));

struct stivale2_struct_tag_pmrs : public stivale2_hdr_tag
{
    uint64_t count;
    stivale2_pmr* pmrs;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_CMDLINE_ID 0xe5e76a1b4597a781
struct stivale2_struct_tag_cmdline : public stivale2_hdr_tag
{
    uint64_t argc; //c string
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_MEMMAP_ID 0x2187f79e8612de07
#define STIVALE2_MMAP_TYPE_USABLE 0x1
#define STIVALE2_MMAP_TYPE_RESERVED 0x2
#define STIVALE2_MMAP_TYPE_ACPI_RECLAIMABLE 0x3
#define STIVALE2_MMAP_TYPE_ACPI_NVS 0x4
#define STIVALE2_MMAP_TYPE_BAD_MEMORY 0x5
#define STIVALE2_MMAP_TYPE_BOOTLOADER_RECLAIMABLE 0x1000
#define STIVALE2_MMAP_TYPE_KERNEL_AND_MODULES 0x1001
#define STIVALE2_MMAP_TYPE_FRAMEBUFFER 0x1002
struct stivale2_mmap_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

//Usable and bootloader entries will always be 4KB aligned, and never overlap. Other segments make no promises.
struct stivale2_struct_tag_memmap : public stivale2_hdr_tag
{
    uint64_t count;
    stivale2_mmap_entry* entries;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID 0x506461d2950408fa
struct stivale2_struct_tag_framebuffer : public stivale2_hdr_tag
{
    uint64_t base;
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint16_t bpp;
    uint8_t memoryModel; //if its not 1, good luck lmao.
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t reserved;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_TEXTMODE_ID 0x38d74c23e0dca893
struct stivale2_struct_tag_textmode : public stivale2_hdr_tag
{
    uint64_t base;
    uint16_t reserved;
    uint16_t rows;
    uint16_t cols;
    uint16_t bytesPerChar;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_EDID_ID 0x968609d7af96b845
struct stivale2_struct_tag_edid : public stivale2_hdr_tag
{
    uint64_t bytesCount;
    uint8_t* bytes;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_TERMINAL_ID 0xc2b3f4c3233b0974
struct stivale2_struct_terminal : public stivale2_hdr_tag
{
    union flags
    {
        uint64_t padding;
        struct
        {
            uint8_t countsPresent : 1; //if set cols and rows are valid
            uint8_t legnthPresent : 1; //if set maxLength is valid
            uint8_t callbackPresent : 1; //set if bootloader likes our callback function
            uint8_t contextControlPresent : 1;
        };
    };

    uint16_t cols;
    uint16_t rows;
    uint64_t termWrite; //address of term_write function: ```void stivale2_term_write(uint64_t ptr, uint64_t length)```, uses sysv abi
    uint64_t maxLength;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_MODULES_ID 0x4b6fe466aade04ce
struct stivale2_module
{
    uint64_t begin;
    uint64_t end;
    uint8_t string[128];//c string, as specified in config file
} __attribute__((packed));
struct stivale2_struct_tag_modules : public stivale2_hdr_tag
{
    uint64_t count;
    stivale2_module* modules;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_RSDP_ID 0x9e1786930a375e78
struct stivale2_struct_tag_rsdp : public stivale2_hdr_tag
{
    uint64_t rsdp;
}__attribute__((packed));

#define STIVALE2_STRUCT_TAG_SMBIOS_ID 0x274bd246c62bf7d1
struct stivale2_struct_tag_smbios : public stivale2_hdr_tag
{
    uint64_t reserved; //meant for future flags
    uint64_t entry32;
    uint64_t entry64;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_EPOCH_ID 0x566a7bed888e1407
struct stivale2_struct_tag_epoch : public stivale2_hdr_tag
{
    uint64_t epoch; //unix epohc at boot, from rtc
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_FIRMWARE_ID 0x359d837855e3858c
struct stivale2_struct_tag_firmware : public stivale2_hdr_tag
{
    uint64_t flags; //0 for uefi, 1 for bios
} __attribute__((packed));

#define STRIVALE2_STRUCT_TAG_EFI_SYSTEM_TABLE_ID 0x4bc5ec15845b558e
struct stivale2_struct_tag_efi_system_table : public stivale2_hdr_tag
{
    uint64_t systemTable;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_KERNEL_FILE_ID 0xe599d90c2975584a
struct stivale2_struct_tag_kernel_file : public stivale2_hdr_tag
{
    uint64_t fileAddress;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_KERNEL_FILE_V2_ID 0x37c13018a02c6ea2
struct stivale2_struct_tag_kernel_file_v2 : public stivale2_hdr_tag
{
    uint64_t fileAddress;
    uint64_t fileSize;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_KERNEL_SLIDE_ID 0xee80847d01506c57
struct stivale2_struct_tag_kernel_slide : public stivale2_hdr_tag
{
    uint64_t offset;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_SMP_ID 0x34d1d96339647025
struct stivale2_smp_info
{
    uint32_t processorId;
    uint32_t lapicId;
    uint64_t targetStack; //once gotoAddress is set, this will be loaded as the stack pointer
    uint64_t gotoAddress; //once atomically written to, will trigger this cpu to start executing code
    uint64_t argument; //useful for passing a pointer to the other cpu
} __attribute__((packed));

struct stivale2_struct_tag_smp : public stivale2_hdr_tag
{
    uint64_t flags; //bit 0 set if x2apic requested before and enabled
    uint32_t bspLapicId; //apic id of the bsp
    uint32_t reserved;
    uint64_t cpuCount;
    stivale2_smp_info* smpInfo; //one entry per processor, including bsp
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_PXE_SERVER_INFO_ID 0x29d1e96239247032
struct stivale2_struct_tag_pxe_server_info : public stivale2_hdr_tag
{
    uint32_t serverIP; //host server ip in network byte order
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_MMIO32_UART_ID 0xb813f9b8dbc78797
struct stivale2_struct_tag_mmio32_uart : public stivale2_hdr_tag
{
    uint64_t address; //where to write 32bit zero-extended serial data stream to.
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_DTB_ID 0xabb29bd49a2833fa
struct stivale2_struct_tag_dtb : public stivale2_hdr_tag
{   
    uint64_t address; //base address of device tree blob
    uint64_t size;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_VMAP_ID 0xb0ed257db18cb58f
struct stivale2_struct_tag_vmap : public stivale2_hdr_tag
{
    uint64_t address; //where physical memory is mapped in higher half.
} __attribute__((packed));
