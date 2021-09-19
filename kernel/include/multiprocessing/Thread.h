#pragma once

#include <stddef.h>
#include <stdint.h>

#define THREAD_DEFAULT_STACK_PAGES 4
#define THREAD_DATA_PROTECT_VALUE 0xDEADC0DEDEADC0DE
#define THREAD_IMPL_DATA_COUNT 16

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
        void* stackMax;
        size_t stackPages;

        uint64_t implementationData[THREAD_IMPL_DATA_COUNT];

        Thread();
        static void ThreadArchInit(Thread* thread, uint64_t entryAddr, bool hasKernelPriv);

    public:
        static Thread* Create(ThreadMainFunction mainFunc, uint8_t priority, uint8_t stackPages = THREAD_DEFAULT_STACK_PAGES);

        void Sleep(size_t millis);
    };
}