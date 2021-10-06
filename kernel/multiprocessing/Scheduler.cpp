#include <multiprocessing/Scheduler.h>
#include <Panic.h>
#include <drivers/APIC.h>
#include <drivers/CPU.h>
#include <drivers/SystemClock.h>
#include <drivers/X86Extensions.h>

extern "C"
{
    //top of stack to operate on
    uint64_t scheduler_nextThreadData;

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
    void IdleThread(void* ignored)
    {
        while (1)
            Drivers::CPU::Halt();
    }
    
    Scheduler localScheduler;
    Scheduler* Scheduler::The()
    {
        return &localScheduler;
    }

    void Scheduler::RegisterThread(Thread* thread)
    {
        threads.PushBack(thread);
    }

    void Scheduler::SelectNext()
    {
        //save the current state if we need to (nullptr == dont save), including the extended state.
        Thread* currentThread = threads[currentIndex];
        if (scheduler_nextThreadData)
        {
            Drivers::X86Extensions::Local()->SaveState(currentThread->extendedSavedState); //TODO: scheduler is platform independent, abstract this pls.
            currentThread->stackTop.raw = scheduler_nextThreadData;
        }

        //selection: run any higher priority tasks first, if they're available to run
        Thread* test = threads.First(); //first should always be idle thread
        currentIndex = 0;
        for (size_t index = 0; index < threads.Size(); index++)
        {
            Thread* selected = threads[index];
            if (selected == nullptr)
                continue;

            //if selected is sleeping, check if we're hit their wake timer, waking them if required
            if (selected->GetState() == ThreadState::Sleeping)
            {
                if (Drivers::SystemClock::The()->GetUptime() >= selected->wakeTime)
                {
                    //wake thread
                    selected->executionState = ThreadState::Running;
                }
            }            
            
            if (selected->priority > test->priority && selected->GetState() == ThreadState::Running)
            {
                currentIndex = index;
                test = threads[index];
            }
        }

        currentThread = test;

        if (!currentThread)
            Panic("No scheduled threads - idle thread has been removed.");
    
        //set new stack top, and load extended state
        scheduler_nextThreadData = currentThread->stackTop.raw;
        Drivers::X86Extensions::Local()->LoadState(currentThread->extendedSavedState);
    }

    void Scheduler::Init()
    {
        threads.Reserve(0x100);
        scheduler_nextThreadData = 0; //we dont want to save the initial thread's state, as we'll never return.
        currentIndex = 0;
        
        //setup idle thread (this will automatically register itself)
        Thread* idleThread = Thread::Create(IdleThread, nullptr, 0);
        idleThread->Start();
    }

    void Scheduler::Yield()
    {
        ISSUE_INTERRUPT_SCHEDULER_YIELD;
    }

    Thread* Scheduler::GetExecutingThread()
    {
        if (currentIndex < threads.Size())
            return threads[currentIndex];
        return nullptr;
    }
}