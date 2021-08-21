#include <multiprocessing/Thread.h>
#include <PageFrameAllocator.h>
#include <PageTableManager.h>
#include <drivers/CPU.h>
#include <multiprocessing/Scheduler.h>

namespace Kernel::Multiprocessing
{
    using Kernel::Drivers::CPU;
    
    static void ThreadMainWrapper(void (threadMain)(void*), void* arg)
    {
        threadMain(arg);
        KernelThread::Exit();
    }
    
    KernelThread* KernelThread::Create(void (*threadMain)(void*), void* arg, uint8_t priority)
    { 
        static uint64_t nextThreadId = 0;
        
        //create stackspace and memory map it
        void* stackStart = nullptr;
        for (int i = 0; i < THREAD_DEFAULT_STACK_PAGES; i++)
        {
            void* nextPage = PageFrameAllocator::The()->RequestPage();
            if (stackStart == nullptr)
                stackStart = nextPage;

            PageTableManager::The()->MapMemory((void*)((uint64_t)stackStart + i * PAGE_SIZE), nextPage, MemoryMapFlags::WriteAllow);
        }

        //place kernel thread data at start of stack, then populate it
        KernelThread* thread = reinterpret_cast<KernelThread*>(stackStart);
        thread->priority = priority;
        thread->threadId = nextThreadId;
        nextThreadId++;
        thread->waitingOnCount = 0;
        thread->stackPages = THREAD_DEFAULT_STACK_PAGES;
        thread->status = ThreadStatus::Sleeping;

        InitKernelThreadData(&thread->data);

        //setup entry point. Again platform specific, as to how execution flows after thread switch
        SetKernelThreadEntry(thread->data, (uint64_t)ThreadMainWrapper, (void*)threadMain, arg);
        SetKernelThreadStack(thread->data, (uint64_t)thread + thread->stackPages * PAGE_SIZE); //place stack at top of allocated space
        SetKernelThreadFlags(thread->data, 0x08, 0x10, 0x202); //kernel code sel offset in gdt, kernel data sel offset in gdt, rflags default

        return thread;
    }

    bool KernelThread::CanRun()
    { 
        return waitingOnCount == 0; 
    }

    void KernelThread::Start()
    { 
        status = ThreadStatus::Running;
        Scheduler::The()->ScheduleThread(this);
    }
    
    void KernelThread::Sleep()
    { }

    void KernelThread::Sleep(int64_t timeout)
    { }
    
    void KernelThread::Wake()
    { }

    void KernelThread::Exit()
    { 
        CPU::DisableInterrupts();

        //remove it from scheduler
        KernelThread* thread = Scheduler::The()->GetCurrentThread();
        Scheduler::The()->UnscheduleThread(thread);
        //free the used memory
        for (int i = 0; i < thread->stackPages; i++)
        {
            PageTableManager::The()->UnmapMemory((void*)((uint64_t)thread + i * PAGE_SIZE));
            PageFrameAllocator::The()->FreePage((void*)((uint64_t)thread + i * PAGE_SIZE));
        }

        CPU::EnableInterrupts();

        Scheduler::The()->Yield();
    }

    uint8_t KernelThread::GetPriority()
    { 
        return priority;
    }

    uint64_t KernelThread::GetId()
    {
        return threadId;
    }

    void KernelThread::CancelTimerWakeup()
    { }
}