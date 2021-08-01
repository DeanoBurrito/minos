#include <multiprocessing/Scheduler.h>
#include <CPU.h>
#include <Interrupts.h>

namespace Kernel::Multiprocessing
{
    extern "C" 
    {
        //Current thread data, so we can access it from assembly routines
        KernelThreadData* Scheduler_currentThreadData;
        
        //Wrapper method to select next thread data to load, callable from assembly
        void Scheduler_SwitchNext()
        {
            Scheduler().The()->SelectNextThreadData();
        }
    }
    
    void IdleMain(void* arg)
    {
        CPU::Halt();
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
        CPU::IssueInterrupt(INTERRUPT_VECTOR_TIMER);
    }

    void Scheduler::SelectNextThreadData()
    {

        KernelThread* nextThread = nullptr;
        //Selection: if the first thread isnt higher priority, we run the nbext one
        if (currentThread != nullptr && currentThread->GetPriority() >= threads.PeekFront()->GetPriority())
            nextThread = threads.Find(currentThread)->next->val;
        
        //Selection: if that didnt work, use the highest priority thread.
        if (nextThread == nullptr || nextThread->GetPriority() < currentThread->GetPriority())
        {
            auto scanHead = threads.Head();
            while (scanHead != nullptr)
            {
                nextThread = scanHead->val;
                if (nextThread->CanRun())
                    break;

                scanHead = scanHead->next;
            }
        }

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