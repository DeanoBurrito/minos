#include <CPU.h>
#include <multiprocessing/Semaphore.h>
#include <multiprocessing/Scheduler.h>

namespace Kernel::Multiprocessing
{
    Semaphore::Semaphore(uint64_t initialCount)
    {
        value = initialCount;
    }

    void Semaphore::Up(uint64_t amount)
    {
        bool interruptsEnabled = CPU::InterruptsEnabled();
        CPU::DisableInterrupts();
        value += amount;

        //wake threads
        Syslib::LinkedListEntry<KernelThread*>* current = waitingThreads.Head();
        while (current != nullptr)
        {
            //wake threads in order, if they were waiting, perhaps they can do something
            auto next = current->next;
            current->val->Wake();
            waitingThreads.Remove(current);
            current = next;
        }

        if (interruptsEnabled)
            CPU::EnableInterrupts(); //restore the flag, only if previously set
    }

    bool Semaphore::Down(uint64_t amount, int64_t timeout)
    {
        bool interruptsEnabled = CPU::InterruptsEnabled();
        CPU::DisableInterrupts();

        while (value < amount)
        {
            //immediate return if we cant take sems
            if (timeout == SEMAPHORE_TIMEOUT_NONE)
                return false;
            
            KernelThread* currentThread = Scheduler::The()->GetCurrentThread();

            //insert based on priority
            Syslib::LinkedListEntry<KernelThread*>* scanHead = waitingThreads.Head();
            while (scanHead != nullptr)
            {
                if (currentThread->GetPriority() >= scanHead->val->GetPriority())
                    break; //found our place, break out
                
                scanHead = scanHead->next;
            }

            //stash the thread after the scanHead from before (either at a reasonable place, or the end)
            if (scanHead == nullptr)
                waitingThreads.InsertAfter(waitingThreads.Tail(), currentThread); //couldnt find a place, place at end
            else
                waitingThreads.InsertAfter(scanHead, currentThread);
            
            if (timeout == SEMAPHORE_TIMEOUT_FOREVER)
                currentThread->Sleep();
            else
            {
                uint64_t oldValue = value;
                currentThread->Sleep(timeout); //sleep for ms, or until woken

                //woke up from either timer or semaphore.up() somewhere else
                if (oldValue != value)
                {
                    //cancel timer wakeup, and continue on
                    currentThread->CancelTimerWakeup();

                    break; 
                }
                else
                {
                    //timer wakeup, clean up and return disappointment
                    waitingThreads.Remove(waitingThreads.FindR(currentThread));

                    if (interruptsEnabled)
                        CPU::EnableInterrupts();
                    return false;
                }
            }
        }

        value -= amount;

        if (interruptsEnabled)
            CPU::EnableInterrupts();

        return true;
    }

    uint64_t Semaphore::Count()
    {
        return value;
    }
}