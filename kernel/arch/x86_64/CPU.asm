.code64
LoadGDT_impl:
    lgdt 0(%rdi)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    pop %rdi
    mov $0x08, %rax
    push %rax
    push %rdi
    retfq

.global LoadGDT_impl
