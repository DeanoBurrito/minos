#pragma once

#include <multiprocessing/Thread.h>
#include <templates/LinkedList.h>

extern "C"
{
    //Declaration so we can use it as a function pointer in c++ 
    void SchedulerTimerInterruptHandler();
}

namespace Kernel::Multiprocessing
{
    class Scheduler
    {
    private:
        sl::LinkedList<KernelThread*> threads;
        KernelThread* currentThread;

    public:
        static Scheduler* The();

        void Init();
        //Co-operatively ends the current quantum, and schedules the next task. Does not remove thread from scheduling queue
        void Yield();

        //Called from interrupt handler to 
        void SelectNextThreadData();
        
        void ScheduleThread(KernelThread* thread);
        void UnscheduleThread(KernelThread* thread);
        KernelThread* GetCurrentThread();
    };
}