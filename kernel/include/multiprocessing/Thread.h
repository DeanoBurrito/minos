#pragma once

#include <stddef.h>
#include <stdint.h>
#include <Memory.h>
#include <collections/List.h>
#include <multiprocessing/ThreadWaitReason.h>

#define THREAD_DEFAULT_STACK_PAGES 4
#define THREAD_DEFAULT_WAIT_LIST_CAPACITY 32
#define THREAD_MIN_STACK_PAGES 2
#define THREAD_DATA_PROTECT_VALUE 0xDEADC0DE

namespace Kernel::Multiprocessing
{
    //forward declarations
    class Scheduler;

    typedef void (*ThreadMainFunction)(void* param);

    enum class ThreadState
    {
        Running,
        Waiting,
        Sleeping,
    };

    class Thread
    {
    friend Scheduler;

    private:
        uint8_t priority;
        size_t stackSize;

        //absolute bottom of stack (highest mem address)
        sl::UIntPtr stackBase;
        //rsp usually
        sl::UIntPtr stackTop;
        //platform specific state that's not stack-stored (x86's (f)xsave stuff)
        void* extendedSavedState;

        ThreadState executionState;
        sl::List<ThreadWaitReason*> waitingReasons;
        uint64_t wakeTime; //since sleeping is so common, keeping track of its state separately is much more efficient than searching the whole list each time.

        Thread();

    public:
        static Thread* Current();
        static Thread* Create(ThreadMainFunction mainFunc, void* arg, uint8_t priority, uint8_t stackPages = THREAD_DEFAULT_STACK_PAGES);

        void Start();
        ThreadState GetState();

        void Sleep(size_t millis);
    };
}