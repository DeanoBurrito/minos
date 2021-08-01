#pragma once

#include <stdint-gcc.h>

#define THREAD_DEFAULT_STACK_PAGES 4

namespace Kernel::Multiprocessing
{
    struct KernelThreadData; //platform specific, see arch folder for implementation details
    class Scheduler;

    enum class ThreadStatus
    {
        Running,
        Sleeping,
        Exited
    };
    
    class KernelThread
    {
    friend Scheduler;
    private:
        //opaque handle to platform specific stuff during thread switching
        KernelThreadData* data;
        uint64_t threadId;
        uint8_t priority;
        uint8_t waitingOnCount;
        uint64_t stackPages;
        ThreadStatus status;

        KernelThread();
        bool CanRun();

    public:
        static KernelThread* Create(void (*threadMain)(void*), void* arg, uint8_t priority);

        void Start();
        void Sleep();
        void Sleep(int64_t timeout);
        void Wake();
        static void Exit();

        uint8_t GetPriority();
        uint64_t GetId();

        void CancelTimerWakeup();
    };

    //platform specific implementations, definitions are in platform folder
    void InitKernelThreadData(KernelThreadData** data);
    void SetKernelThreadEntry(KernelThreadData* data, uint64_t mainAddr, void* arg0, void* arg1);
    void SetKernelThreadStack(KernelThreadData* data, uint64_t base);
    void SetKernelThreadFlags(KernelThreadData* data, uint64_t codeSegment, uint64_t dataSegment, uint64_t flags); //TODO: platform agnostic way of naming these
}