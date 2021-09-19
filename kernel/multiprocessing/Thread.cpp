#include <multiprocessing/Thread.h>
#include <multiprocessing/Scheduler.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Platform.h>
#include <Memory.h>
#include <InterruptScopeGuard.h>

namespace Kernel::Multiprocessing
{
    Thread* Thread::Create(ThreadMainFunction mainFunc, uint8_t nPriority, uint8_t nStackPages)
    {
        InterruptScopeGuard intGuard = InterruptScopeGuard();
        nStackPages = nStackPages > 1 ? nStackPages : 1; //at least allocate 1 page
        
        //TODO: when processes are added, let them assign the stack address for a thread
        //NOTE: This can currently overwrite existing page maps if we're not careful. This will be solved when processes
        //      are implemented, as they'll separate virtual memory space.
        void* lastPage = nullptr;
        for (size_t i = 0; i < nStackPages; i++)
        {
            void* stackPage = PageFrameAllocator::The()->RequestPage();

            if (!lastPage)
                PageTableManager::The()->MapMemory(stackPage, stackPage, MemoryMapFlags::WriteAllow);
            else
                PageTableManager::The()->MapMemory((void*)((uint64_t)lastPage + PAGE_SIZE), stackPage, MemoryMapFlags::WriteAllow);
            lastPage = stackPage;
        }

        lastPage = (void*)((uint64_t)lastPage + PAGE_SIZE); //peak programmer. Amazing.
        Thread* thread = reinterpret_cast<Thread*>((uint64_t)lastPage - sizeof(Thread) - 8);
        sl::memset(thread, 0, sizeof(Thread));

        //stuff known crazy values above and below thread, just incase of erros adjusting stack. We can detect it.
        *(uint64_t*)((uint64_t)lastPage - 8) = THREAD_DATA_PROTECT_VALUE;
        *(uint64_t*)((uint64_t)thread - 8) = THREAD_DATA_PROTECT_VALUE;

        thread->priority = nPriority;
        thread->stackMax = reinterpret_cast<void*>((uint64_t)thread - 16); //8 bytes for deadcode, 8 bytes of zeros for stopping stack traces.
        thread->stackPages = nStackPages;

        ThreadArchInit(thread, (uint64_t)mainFunc, true);

        return thread;
    }

    Thread::Thread()
    {}

    void Thread::Sleep(size_t millis)
    {}
}