#include <drivers/CPU.h>

namespace Kernel::Drivers
{
    InterruptScopeGuard::InterruptScopeGuard()
    {
        previouslyEnabled = CPU::InterruptsEnabled();
        CPU::DisableInterrupts();
    }

    InterruptScopeGuard::~InterruptScopeGuard()
    {
        if (previouslyEnabled)
            CPU::EnableInterrupts();
    }

    void InterruptScopeGuard::NeverReset()
    {
        previouslyEnabled = false;
    }
}