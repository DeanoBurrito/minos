#include <multiprocessing/Scheduler.h>
#include <drivers/CPU.h>
#include <Interrupts.h>

namespace Kernel::Multiprocessing
{
    using Kernel::Drivers::CPU;
    
    extern "C" 
    {
        //Current thread data, so we can access it from assembly routines
        KernelThreadData* Scheduler_currentThreadData;
        
        //Wrapper method to select next thread data to load, callable from assembly
        void Scheduler_SwitchNext()
        {
            Scheduler::The()->SelectNextThreadData();
        }
    }
    
    void IdleMain(void* arg)
    {
        while (true)
        {
            //Dont work too hard
            CPU::Halt();
        }
    }
    
    Scheduler schedulerInstance;
    Scheduler* Scheduler::The()
    {
        return &schedulerInstance;
    }

    void Scheduler::Init()
    {
        KernelThread* idleThread = KernelThread::Create(IdleMain, nullptr, 0); //lowest priority possible
        idleThread->Start();
        currentThread = nullptr;
        Scheduler_currentThreadData = nullptr;

        //TODO: setup quantum to a known value, check what timer sources we can install ourselves into.
    }

    void Scheduler::Yield()
    {
        //manually trigger timer handler
        //ISSUE_INTERRUPT(INTERRUPT_VECTOR_TIMER);
        ISSUE_INTERRUPT_SCHEDULER_YIELD;
    }

    //NOTE: This is called from within interrupt handler, dont do anything too crazy here
    void Scheduler::SelectNextThreadData()
    {
        KernelThread* nextThread = threads.PeekFront();

        //Thread selection
        nextThread = threads.PeekBack(); //TODO: actual scheduling

        //next thread has been selected, set pointer and return to assembly to load registers
        Scheduler_currentThreadData = nextThread->data;
        currentThread = nextThread;
    }

    void Scheduler::ScheduleThread(KernelThread* thread)
    {
        //run through list until we find a spot, or add it to the end.
        auto scanHead = threads.Head();
        while (scanHead != nullptr)
        {
            if (scanHead->val->GetPriority() >= thread->GetPriority())
                break;

            scanHead = scanHead->next;
        }

        if (scanHead == nullptr)
            threads.InsertAfter(threads.Tail(), thread);
        else
            threads.InsertAfter(scanHead, thread);
    }

    void Scheduler::UnscheduleThread(KernelThread* thread)
    {
        threads.Remove(threads.Find(thread));
        
        if (currentThread == thread)
            currentThread = nullptr;
    }

    KernelThread* Scheduler::GetCurrentThread()
    { 
        return currentThread;
    }
}