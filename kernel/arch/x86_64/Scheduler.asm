.code64

.extern scheduler_nextThreadData
.extern scheduler_selectNext
.extern scheduler_sendEOI

.global scheduler_HandleInterrupt

scheduler_HandleInterrupt:
    #save rdi and then rax to give us some scratch registers
    push %rdi
    mov scheduler_nextThreadData, %rdi

    #check if current thread needs its registers saved
    test %rdi, %rdi
    jz NoSaveRegs

    #push all regs onto the stack
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    #skip rdi as its actually pushed first
    #skip rsp as we have to save it externally, otherwise we could never come back here
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    #now save rsp so we can return to this data
    mov %rsp, scheduler_nextThreadData

    jmp RestoreRegs

NoSaveRegs:
    #just clean the stack in that case
    #NOTE: this actually does nothing, because we havent saved the current stack pointer, so we cant return here.
    pop %rax

RestoreRegs:
    #now that we dont need to worry about corrupting registers, we can call into cpp-land
    call scheduler_selectNext
    call scheduler_sendEOI

    #load fresh thread info, start operating on their stack
    mov scheduler_nextThreadData, %rsp

    #load new stack pointer, pop all regs back into place, and issue iret (previous interrupt_frame should be in place)
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    pop %rdi

    iretq
