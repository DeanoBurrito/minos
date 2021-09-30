#pragma once

#include <stddef.h>
#include <stdint.h>

#define THREAD_DEFAULT_STACK_PAGES 4
#define THREAD_MIN_STACK_PAGES 2
#define THREAD_DATA_PROTECT_VALUE 0xDEADC0DE

namespace Kernel::Multiprocessing
{
    //forward declarations
    class Scheduler;

    typedef void (*ThreadMainFunction)(void* param);

    class Thread
    {
    friend Scheduler;

    private:
        uint8_t priority;
        size_t stackSize;

        //absolute bottom of stack (highest mem address)
        void* stackBase;
        //rsp usually
        void* stackTop;
        //platform specific state that's not stack-stored (x86's (f)xsave stuff)
        void* extendedSavedState;

        Thread();

    public:
        static Thread* Create(ThreadMainFunction mainFunc, void* arg, uint8_t priority, uint8_t stackPages = THREAD_DEFAULT_STACK_PAGES);

        void Start();
    };
}