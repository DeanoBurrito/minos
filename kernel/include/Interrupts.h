#pragma once

struct interrupt_frame;

namespace InterruptHandlers
{
    __attribute__((interrupt)) void DoubleFault(interrupt_frame* frame);
    __attribute__((interrupt)) void GeneralProtectionFault(interrupt_frame* frame);
    __attribute__((interrupt)) void PageFault(interrupt_frame* frame);

    __attribute__((interrupt)) void PS2KeyboardHandler(interrupt_frame* frame);
    __attribute__((interrupt)) void TimerHandler(interrupt_frame* frame);
}