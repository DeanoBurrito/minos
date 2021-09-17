.code64
.text

#buffer of uint64s big enough to contain all registers
.extern capturedRegistersBuffer
.global CaptureRegisters_impl

CaptureRegisters_impl:
    #save rdi, and use it to hold address of our buffer
    push %rdi
    mov (capturedRegistersBuffer), %rdi
    
    #capture rax, rbx, rcx, rdx, rsi
    mov %rax, 0x00(%rdi)
    mov %rbx, 0x08(%rdi)
    mov %rcx, 0x10(%rdi)
    mov %rdx, 0x18(%rdi)
    mov %rsi, 0x20(%rdi)

    #capture rdi
    pop %rax
    mov %rax, 0x28(%rdi)

    #capture r8-r15
    mov %r8, 0x30(%rdi)
    mov %r9, 0x38(%rdi)
    mov %r10, 0x40(%rdi)
    mov %r11, 0x48(%rdi)
    mov %r12, 0x50(%rdi)
    mov %r13, 0x58(%rdi)
    mov %r14, 0x60(%rdi)
    mov %r15, 0x68(%rdi)

    #TODO: look into how we can examine stack frames
    #capture rbp, rsp and rip (of previous frame - not the current function's data)
    mov $0, 0x70(%rdi)
    mov $0, 0x78(%rdi)
    mov $0, 0x80(%rdi)

    #capture flags and segments
    pushf
    pop %rax
    mov %rax, 0x88(%rdi)
    mov %cs, 0x90(%rdi)
    mov %ds, 0x98(%rdi)
    mov %ss, 0xA0(%rdi)
    mov %es, 0xA8(%rdi)
    mov %fs, 0xB0(%rdi)
    mov %gs, 0xB8(%rdi)

    #capture control regs
    mov %cr0, %rax
    mov %rax, 0xC0(%rdi)
    mov %cr2, %rax
    mov %rax, 0xC8(%rdi)
    mov %cr3, %rax
    mov %rax, 0xD0(%rdi)
    mov %cr4, %rax
    mov %rax, 0xD8(%rdi)

    sidt 0xE0(%rdi)
    sgdt 0xF0(%rdi)

    #reset rax and rdi to previous values (since i'm not sure what calling convention we're using - just restore them)
    mov 0x28(%rdi), %rax
    push %rax
    mov 0x00(%rdi), %rax
    pop %rdi
    ret
