.code32
.extern MultibootMain
.global OldSchoolCoolMain
.global error

.set MB_MAGIC, 0x1BADB002
.set MB_LOAD_PAGE_ALIGNED, 1 << 0
.set MB_REQUEST_MEMORY_INFO, 1 << 1
.set MB_REQUEST_VIDEO_INFO, 1 << 2
.set MB_OVERRIDE_LOAD_INFO, 1 << 16

.set MB_FLAGS, MB_LOAD_PAGE_ALIGNED | MB_REQUEST_MEMORY_INFO
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.align 4
.section .multiboot
    #multiboot header
    .long MB_MAGIC
    .long MB_FLAGS
    .long MB_CHECKSUM
    
    #section load address overrides (if flag is set, otherwise ignored)
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    
    #video mode requested settings - 0 means dont care
    .long 0
    .long 0
    .long 0

.align 16
.section .text
OldSchoolCoolMain:
    #setup stack, reset flags, prime stack for cdecl call and jump into main
    movl $(stack + 0x2000), %esp
    movl %esp, %ebp

    #eax should contain 0x2BADB002, ebx is the address of multiboot struct.
    pushl %ebx
    pushl %eax

    call MultibootMain
    #if we return, fall through to error code below

error:
    cli
    mov $0xDEADC0DE, %eax
    mov $0xDEADC0DE, %ebx
    mov $0xDEADC0DE, %ecx
    mov $0xDEADC0DE, %edx
loop:
    hlt
    jmp loop

.comm stack, 0x2000
