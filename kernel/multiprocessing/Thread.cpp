#include <multiprocessing/Thread.h>
#include <drivers/X86Extensions.h>
#include <PageTableManager.h>
#include <PageFrameAllocator.h>
#include <Platform.h>
#include <Memory.h>
#include <InterruptScopeGuard.h>
#include <KLog.h>

namespace Kernel::Multiprocessing
{
    //just a convinience function
    __attribute__((always_inline)) inline void StackPush(sl::UIntPtr sp, uint64_t word)
    {
        sp.raw -= 8;
        sl::MemWrite(sp, word);
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

        nextStackBegin += nextStackBegin / 100; //arbitrary, placeholder until processes

        for (size_t count = 1; count < stackPages; count++)
        {
            sl::UIntPtr nextPage = PageFrameAllocator::The()->RequestPage();
            PageTableManager::The()->MapMemory(lastPage.ptr, nextPage.ptr, MemoryMapFlags::WriteAllow);
            lastPage.raw += PAGE_SIZE;
        }

        //create thread structure on the new stack, and surround it with known protect values.
        sl::UIntPtr sp = lastPage.raw - sizeof(uint64_t);
        sl::MemWrite(sp, (uint64_t)THREAD_DATA_PROTECT_VALUE);

        sp.raw -= sizeof(Thread);
        Thread* thread = reinterpret_cast<Thread*>(sp.ptr);

        sp.raw -= sizeof(uint64_t);
        sl::MemWrite(sp, (uint64_t)THREAD_DATA_PROTECT_VALUE);

        //write 2 zero words to top of stack, to stop any stack traces.
        sp.raw -= sizeof(uint64_t) * 2;
        sl::MemWrite(sp.raw + 8, (uint64_t)0);
        sl::MemWrite(sp, (uint64_t)0);

        //now that we have a thread structure, keep track of anything we need to.
        thread->priority = priority;
        thread->stackSize = stackPages * PAGE_SIZE - (sizeof(uint64_t) * 4) - sizeof(Thread);
        thread->stackBase = sp.ptr;
        size_t xStateBufferSize = Drivers::X86Extensions::Local()->GetStateBufferSize(); //TODO: would be nice to also store this above the thread on the stack?
        thread->extendedSavedState = new uint8_t[xStateBufferSize];
        
        //prep for scheduler: setup iretq frame, then push blank regs onto stack
        uint64_t expectedSP = sp.raw - 5 - 15; //+5 for stack frame, +15 for registers (rsp not saved)
        StackPush(sp, 0x10); //stack selector (data segment)
        StackPush(sp, expectedSP); //stack pointer
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
        StackPush(sp, (uint64_t)thread->stackTop); //rbp (same as rsp)

        for (size_t count = 0; count < 8; count++)
            StackPush(sp, 0); //r15 -> r8 (inclusive)
        
        thread->stackTop = sp.ptr;
        //some sanity checks
        if (sp.raw != expectedSP)
            LogError("Thread stack top is not where expected!");
        if (expectedSP != (uint64_t)thread->stackBase - (5 + 15))
            LogError("Thread stack size is incorrect");

        return thread;
    }

    Thread::Thread()
    {}

    void Thread::Start()
    {}
}