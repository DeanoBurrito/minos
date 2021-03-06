#include <multiprocessing/Thread.h>
#include <drivers/X86Extensions.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Platform.h>
#include <Memory.h>
#include <InterruptScopeGuard.h>
#include <multiprocessing/Scheduler.h>
#include <drivers/SystemClock.h>
#include <KLog.h>

namespace Kernel::Multiprocessing
{
    //just a convinience function
    __attribute__((always_inline)) inline void StackPush(sl::UIntPtr &sp, uint64_t word)
    {
        sp.raw -= 8;
        sl::MemWrite(sp, word);
    }

    Thread* Thread::Current()
    {
        return Scheduler::The()->GetExecutingThread();
    }
    
    Thread* Thread::Create(ThreadMainFunction mainFunc, void* arg, uint8_t priority, uint8_t stackPages)
    {
        static uint64_t nextStackBegin = 0x2'000'000'000; //TODO: have owner process give thread a starting stack address
        
        volatile InterruptScopeGuard intGuard {};
        stackPages = stackPages < THREAD_MIN_STACK_PAGES ? THREAD_MIN_STACK_PAGES : stackPages; //lower bound on memory allocated for stack
        
        //setup initial page, and next stack start in this space
        sl::UIntPtr lastPage = PageFrameAllocator::The()->RequestPage();
        PageTableManager::The()->MapMemory((void*)nextStackBegin, lastPage.ptr, MemoryMapFlags::WriteAllow); 
        lastPage.raw += PAGE_SIZE;

        nextStackBegin += stackPages * PAGE_SIZE; //arbitrary, placeholder until processes

        for (size_t count = 1; count < stackPages; count++)
        {
            sl::UIntPtr nextPage = PageFrameAllocator::The()->RequestPage();
            PageTableManager::The()->MapMemory(lastPage.ptr, nextPage.ptr, MemoryMapFlags::WriteAllow);
            lastPage.raw += PAGE_SIZE;
        }

        //create thread structure on the new stack, and surround it with known protect values.
        sl::UIntPtr sp = lastPage.raw;
        StackPush(sp, THREAD_DATA_PROTECT_VALUE);

        sp.raw -= sizeof(Thread);
        Thread* thread = reinterpret_cast<Thread*>(sp.ptr);
        sl::memset(thread, 0, sizeof(Thread)); //default init all thread data
        StackPush(sp, THREAD_DATA_PROTECT_VALUE);

        //write 2 zero words to top of stack, to stop any stack traces.
        StackPush(sp, 0);
        StackPush(sp, 0);

        //now that we have a thread structure, keep track of anything we need to.
        thread->priority = priority;
        //thread->waitingReasons.Reserve(THREAD_DEFAULT_WAIT_LIST_CAPACITY); //TODO: this crashes? but only when using larger numbers
        thread->stackSize = stackPages * PAGE_SIZE - (sizeof(uint64_t) * 4) - sizeof(Thread);
        thread->stackBase = sp.ptr;

        //NOTE: this can be a huge datastructure on later CPUs (ZMM regs are 512 bits wide), so storing it on the stack is a bit of waste.
        //TODO: a runtime-sized slab allocator for these. Since we cant know until we actually query the CPU how much state we'll need to save.
        size_t xStateBufferSize = Drivers::X86Extensions::Local()->GetStateBufferSize();
        thread->extendedSavedState = new uint8_t[xStateBufferSize];
        sl::memset(thread->extendedSavedState, 0, xStateBufferSize);
        
        //prep for scheduler: setup iretq frame, then push blank regs onto stack
        uint64_t execStack = sp.raw;
        StackPush(sp, 0x10); //stack selector (data segment)
        StackPush(sp, execStack); //stack pointer
        StackPush(sp, 0x202); //rflags: interrupts enabled, and bit 1 is always high.
        StackPush(sp, 0x8); //code selector
        StackPush(sp, (uint64_t)mainFunc); //rip: address to return execution to

        //TODO: this is specific to x86_64
        //We're using sysv abi (64bit) for our calling convention here
        StackPush(sp, (uint64_t)arg); //rdi last (1st arg)
        StackPush(sp, 0); //rax
        StackPush(sp, 0); //rbx
        StackPush(sp, 0); //rcx
        StackPush(sp, 0); //rdx
        StackPush(sp, 0); //rsi 
        StackPush(sp, execStack); //rbp (same as rsp)

        for (size_t count = 0; count < 8; count++)
            StackPush(sp, 0); //r15 -> r8 (inclusive)
        
        thread->stackTop = sp.ptr;
        thread->executionState = ThreadState::Running;

        return thread;
    }

    Thread::Thread()
    {}

    void Thread::Start()
    {
        //register ourselves with scheduler
        Scheduler::The()->RegisterThread(this);
    }

    ThreadState Thread::GetState()
    {
        return executionState;
    }

    void Thread::Sleep(size_t millis)
    {
        //calculate wake-time in ms, stash it and then tell scheduler to add us to the sleep list
        wakeTime = Drivers::SystemClock::The()->GetUptime() + millis;
        executionState = ThreadState::Sleeping;

        Scheduler::The()->Yield();
    }
}