#pragma once

namespace Kernel
{
    class InterruptScopeGuard
    {
    private:
        bool previouslyEnabled;
    public:
        InterruptScopeGuard();
        ~InterruptScopeGuard();

        //never resets the interrupt flag on destruction
        void NeverReset();

        InterruptScopeGuard(const InterruptScopeGuard&) = delete;
        void operator=(const InterruptScopeGuard&) = delete;
        InterruptScopeGuard(InterruptScopeGuard&&) = delete;
        void operator=(InterruptScopeGuard&&) = delete;
    };
}
