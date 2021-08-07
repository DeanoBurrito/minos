#include <CPU.h>

namespace Kernel
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