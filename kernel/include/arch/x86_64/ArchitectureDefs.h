#pragma once

#include <stdint.h>

/*
    This file contains any platform specific magic numbers. I'd rather keep them in one place, rather than
    all over the code base.
*/

//NativePtr - required for compilation
#define NativePtr uint64_t

//General magic numbers
#define PAGE_SIZE 0x1000

//MSRs
#define X86_MSR_EFER 0xC0000080

//cpuid leaf for xsave feature sets
#define X86_CPUID_XSAVE 0xD

//IA32e (long mode/64bit/whatever name they cant agree on) interrupt frame as per spec. This is what iret will process
struct interrupt_frame
{
    uint64_t ss;
    uint64_t rsp;
    uint64_t rflags;
    uint64_t cs;
    uint64_t rip;
};

//minos x64 specific frame, we add a few extras, and always push an error code (even if unused)
struct MinosInterruptFrame
{
    uint64_t vectorNumber;
    uint64_t errorCode;

    uint64_t rInstructionPointer;
    uint64_t codeSelector;
    uint64_t rFlags;
    uint64_t rStackPointer;
    uint64_t stackSelector;
};
