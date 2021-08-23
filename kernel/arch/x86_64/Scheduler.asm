.code64

.extern Scheduler_currentThreadData
.extern Scheduler_SwitchNext
.extern Asm_SendEOI
.global SchedulerTimerInterruptHandler

#Note: This code is tightly coupled with KernelThreadData.cpp. Any changes in either file must be checked against the other.
SchedulerTimerInterruptHandler:
    # save rdi to the stack, then load rdi with the pointer to our current thread data
    push %rdi
    mov (Scheduler_currentThreadData), %rdi

    #only save current thread data if we have somewhere to save it (its not nullptr)
    test %rdi, %rdi
    jz SkipSave

    # save rax, so we can use it, then pop and store rdi
    mov %rax, 0x28(%rdi)
    pop %rax
    mov %rax, 0x50(%rdi)

    # Save rip, cs, rflags, rsp, ss
    pop %rax
    mov %rax, 0x20(%rdi)
    pop %rax
    mov %rax, 0x18(%rdi)
    pop %rax
    mov %rax, 0x10(%rdi)
    pop %rax
    mov %rax, 0x08(%rdi)
    pop %rax
    mov %rax, 0x00(%rdi)

    # save (!rax) rbx, rcx, rdx, rsi, (!rdi), rbp
    mov %rbx, 0x30(%rdi)
    mov %rcx, 0x38(%rdi)
    mov %rdx, 0x40(%rdi)
    mov %rsi, 0x48(%rdi)
    mov %rbp, 0x58(%rdi)

    # save r8, r9, r10, r11, r12, r13, r14, r15
    mov %r8, 0x60(%rdi)
    mov %r9, 0x68(%rdi)
    mov %r10, 0x70(%rdi)
    mov %r11, 0x78(%rdi)
    mov %r12, 0x80(%rdi)
    mov %r13, 0x88(%rdi)
    mov %r14, 0x90(%rdi)
    mov %r15, 0x98(%rdi)

    # save ds, es, gs, fs
    mov %ds, 0xA0(%rdi)
    mov %es, 0xA8(%rdi)
    mov %fs, 0xB0(%rdi)
    mov %gs, 0xB8(%rdi)

    # Now that registers have been saved, we're safe to call other routines, without fear of trashing anything
    jmp LoadNextThread

SkipSave:
    #no need to save previous regs, just need to clean %rdi from the stack
    pop %rdi

LoadNextThread:
    # Call switchnext() to load the next thread
    call Scheduler_SwitchNext

    # Send EOI before we load anything back onto the stack, as this will corrupt it
    call Asm_SendEOI

    mov (Scheduler_currentThreadData), %rdi

    #load ss, rsp, rflags, cs, rip onto the stack
    push 0x00(%rdi)
    push 0x08(%rdi)
    push 0x10(%rdi)
    push 0x18(%rdi)
    push 0x20(%rdi)

    # load rax, rbx, rcx, rdx, rsi, (!rdi), rbp
    mov 0x28(%rdi), %rax
    mov 0x30(%rdi), %rbx
    mov 0x38(%rdi), %rcx
    mov 0x40(%rdi), %rdx
    mov 0x48(%rdi), %rsi
    mov 0x58(%rdi), %rbp

    #load r8, r9, r10, r11, r12, r13, r14, r15
    mov 0x60(%rdi), %r8
    mov 0x68(%rdi), %r9
    mov 0x70(%rdi), %r10
    mov 0x78(%rdi), %r11
    mov 0x80(%rdi), %r12
    mov 0x88(%rdi), %r13
    mov 0x90(%rdi), %r14
    mov 0x98(%rdi), %r15

    #load ds, es, fs, gs, then finally rdi now that we're done with it
    mov 0xA0(%rdi), %ds
    mov 0xA8(%rdi), %es
    mov 0xB0(%rdi), %fs
    mov 0xB8(%rdi), %gs

    mov 0x50(%rdi), %rdi

    iretq
