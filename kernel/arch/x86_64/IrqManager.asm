.code64

#if non-zero, the address to save rsp before jumping to cppland, and read rsp from after returning, otherwise rsp is not saved
.extern irqSaveStack
.extern IrqManager_Dispatch

.global IrqDefaultEntry
.global IrqDefaultEntry_End

    #we'll also push a dummy error code here when populating idt, if needed
IrqDefaultEntry:
    #we'll manually insert the 'push $vector-numer' op here when populating idt (thanks StaticSaga for the cursed ideas)

    #save all registers
    push %rax
    push %rbx
    push %rcx
    push %rdx

    push %rsi
    push %rdi
    #skip rsp, as we save it externally. Otherwise we'd never be able to come back here
    push %rbp

    push %r8
    push %r9
    push %r10
    push %r11

    push %r12
    push %r13
    push %r14
    push %r15

    mov %ds, %rax
    push %rax
    mov %es, %rax
    push %rax
    mov %fs, %rax
    push %rax
    mov %gs, %rax
    push %rax

    #if irqSaveStack is valid, save the current stack top there
    mov irqSaveStack, %rdi
    test %rdi, %rdi
    jz IrqMakeCall
    mov %rsp, irqSaveStack

IrqMakeCall:
    #put address of our interrupt frame into rdi, make the call (it will take care of EOI for us)
    mov %rsp, %rdi
    sub $0x78, %rdi

    call IrqManager_Dispatch

    #like before, if irqSaveStack is valid, we use it for the new stack top
    mov irqSaveStack, %rdi
    test %rdi, %rdi
    jz IrqRestoreRegs
    mov irqSaveStack, %rsp

IrqRestoreRegs:
    #restore regs
    pop %rax
    mov %rax, %gs
    pop %rax
    mov %rax, %fs
    pop %rax
    mov %rax, %es
    pop %rax
    mov %rax, %ds

    pop %r15
    pop %r14
    pop %r13
    pop %r12

    pop %r11
    pop %r10
    pop %r9
    pop %r8

    pop %rbp
    #skip rsp
    pop %rdi
    pop %rsi

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    #consume error code and vector number on stack, so iret gets correct values
    add $16, %rsp

    iretq

IrqDefaultEntry_End: #just here so we know when to stop copying
