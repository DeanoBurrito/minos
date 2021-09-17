#include <InterruptScopeGuard.h>
#include <drivers/CPU.h>

namespace Kernel
{
    InterruptScopeGuard::InterruptScopeGuard()
    {
        previouslyEnabled = Drivers::CPU::InterruptsEnabled();
        Drivers::CPU::DisableInterrupts();
    }

    InterruptScopeGuard::~InterruptScopeGuard()
    {
        if (previouslyEnabled)
            Drivers::CPU::EnableInterrupts();
    }

    void InterruptScopeGuard::NeverReset()
    {
        previouslyEnabled = false;
    }
}