OUTPUT_FORMAT(elf64-x86-64)
ENTRY(StivaleMain)

SECTIONS
{
    .stivale2hdr :
    {
        *(.stivale2hdr)
    }
    
    .text : ALIGN(4096)
    {
        *(.text)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
}