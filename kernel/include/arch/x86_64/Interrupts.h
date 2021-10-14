#pragma once

#include <stdint.h>
#include <Platform.h>

//These are defined by intel spec, and we cant change
#define INTERRUPT_VECTOR_DOUBLE_FAULT 0x08
#define INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_VECTOR_PAGE_FAULT 0x0E
#define INTERRUPT_VECTOR_FPU_ERROR 0x10
#define INTERRUPT_VECTOR_SIMD_ERROR 0x13

//Software defined, change these as we need.
#define INTERRUPT_VECTOR_PS2KEYBOARD 0x21
#define INTERRUPT_VECTOR_TIMER 0x30
#define INTERRUPT_VECTOR_TIMER_CALIBRATE 0x23

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame, uint64_t errorCode);
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame, uint64_t errorCode);
    __attribute__((interrupt)) void PageFault(interrupt_frame* frame, uint64_t errorCode);
    __attribute__((interrupt)) void FPUError(interrupt_frame* frame);
    __attribute__((interrupt)) void SIMDError(interrupt_frame* frame);

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame);
    __attribute__((interrupt)) void SystemClockHandler(interrupt_frame* frame);
}