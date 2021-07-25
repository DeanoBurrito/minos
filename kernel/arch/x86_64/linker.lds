OUTPUT_FORMAT(elf64-x86-64)
ENTRY(KernelMain)

SECTIONS
{
    _KernelStart = .;
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
    _KernelEnd = .;
}