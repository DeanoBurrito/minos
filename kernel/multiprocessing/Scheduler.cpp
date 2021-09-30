#include <multiprocessing/Scheduler.h>
#include <drivers/APIC.h>
#include <drivers/CPU.h>
#include <drivers/X86Extensions.h>

extern "C"
{
    //top of stack to operate on
    uint64_t* scheduler_nextThreadData;

    void scheduler_selectNext() 
    {
        //ah yes, thanks c++. Really keeping things simple.
        Kernel::Multiprocessing::Scheduler::The()->SelectNext();
    }

    void scheduler_sendEOI() 
    {
        Kernel::Drivers::APIC::Local()->SendEOI();
    }
}

namespace Kernel::Multiprocessing
{
    void IdleThread()
    {
        while (1)
            Drivers::CPU::Halt();
    }
    
    Scheduler localScheduler;
    Scheduler* Scheduler::The()
    {
        return &localScheduler;
    }

    void Scheduler::SelectNext()
    {
        //save extended state, move current thread back into queue, load next thread's data
        Drivers::X86Extensions::Local()->SaveState(currentThread->extendedSavedState); //TODO: scheduler is platform independent, abstract this pls.

        //selection:

        Drivers::X86Extensions::Local()->LoadState(currentThread->extendedSavedState);
    }

    void Scheduler::Init()
    {
        //TODO: idle thread so we always have a fallback
    }

    void Scheduler::Yield()
    {
        ISSUE_INTERRUPT_SCHEDULER_YIELD;
    }
}