.code32
.extern MultibootMain
.global OldSchoolCoolMain

.align 4
.section .multiboot
    .long 0x1BADB002
    .long 0x7
    .long 0xE4524FF7
    .long 0

    .long 0
    .long 0
    .long 0
    .long 0
    
    .long 0
    .long 0
    .long 0

.align 16
.section .text
OldSchoolCoolMain:
    #setup stack, reset flags, prime stack for cdecl call and jump into main
    movl $(stack + 0x2000), %esp

    pushl $0
    popf

    pushl %ebx
    pushl %eax

    call MultibootMain
    #if we return, fall through to error code below

error:
    movl 0xDEADC0DE, %eax
    movl 0xDEADC0DE, %ebx
    movl 0xDEADC0DE, %ecx
    movl 0xDEADC0DE, %edx
loop:
    hlt
    jmp loop

.comm stack, 0x2000
