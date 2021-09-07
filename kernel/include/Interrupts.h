#pragma once

#include <stdint-gcc.h>

//These are defined by intel spec, and we cant change
#define INTERRUPT_VECTOR_DOUBLE_FAULT 0x08
#define INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_VECTOR_PAGE_FAULT 0x0E

//Software defined, change these as we need.
#define INTERRUPT_VECTOR_PS2KEYBOARD 0x21
#define INTERRUPT_VECTOR_TIMER 0x30
#define INTERRUPT_VECTOR_TIMER_CALIBRATE 0x23

//NOTE: this is ISA specific. Currently this is setup for x86_64
struct interrupt_frame
{
    uint64_t ss;
    uint64_t rsp;
    uint64_t rflags;
    uint64_t cs;
    uint64_t rip;
};

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame, uint64_t errorCode);
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame, uint64_t errorCode);
    __attribute__((interrupt)) void PageFault(interrupt_frame* frame, uint64_t errorCode);

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame);
    __attribute__((interrupt)) void DefaultTimerHandler(interrupt_frame* frame);
}