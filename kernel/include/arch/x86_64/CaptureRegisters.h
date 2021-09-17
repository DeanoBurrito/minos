#pragma once

#include <stdint.h>

namespace Kernel
{
    struct CapturedRegisters
    {
        uint64_t rax, rbx, rcx, rdx, rsi, rdi;
        uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
        uint64_t rbp, rsp, rip;
        uint64_t rflags, cs, ds, ss, es, fs, gs;
        uint64_t cr0, cr2, cr3, cr4;
        uint16_t idtLimit;
        uint64_t idtBase;
        uint8_t reserved0[6];
        uint16_t gdtLimit;
        uint64_t gdtBase;
        uint8_t reserved1[6];
    }__attribute__((packed));

    void CaptureRegisters(CapturedRegisters* stash);
}
