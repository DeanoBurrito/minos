#pragma once

/*
    This file contains any platform specific magic numbers. I'd rather keep them in one place, rather than
    all over the code base.
*/

#define NativePtr uint64_t

//General magic numbers
#define PAGE_SIZE 0x1000

//MSRs
#define X86_MSR_EFER 0xC0000080

//cpuid leaf for xsave feature sets
#define X86_CPUID_XSAVE 0xD
