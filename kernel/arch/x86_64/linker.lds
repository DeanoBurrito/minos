OUTPUT_FORMAT(elf64-x86-64)
ENTRY(KernelMain)

PHDRS
{
    null    PT_NULL     FLAGS(0);
    text    PT_LOAD     FLAGS((1 << 0) | (1 << 2)); /* read + exec */
    rodata  PT_LOAD     FLAGS((1 << 2));           /* read only */
    data    PT_LOAD     FLAGS((1 << 1) | (1 << 2)); /* write + read */
    dynamic PT_DYNAMIC  FLAGS((1 << 1) | (1 << 2)); /* same as above, for pie */
}

SECTIONS
{   
    . = 0; /* for debugging purposes right now, will move to higher half soon */
    
    .text : ALIGN(4096)
    {
        *(.text)
        *(.text.*)
    } :text

    .data : ALIGN(4096)
    {
        *(.data)
    } :data

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    } :rodata

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    } :data

    .dynamic : ALIGN(4096)
    {
        *(.dynamic)
    } :data :dynamic
}