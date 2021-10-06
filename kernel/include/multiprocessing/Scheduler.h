#pragma once

#include <multiprocessing/Thread.h>
#include <collections/List.h>

extern "C"
{
    void scheduler_HandleInterrupt();
}

namespace Kernel::Multiprocessing
{
    class Scheduler
    {
    private:
        sl::List<Thread*> threads; //TODO: shouldnt be a pointer, emplace within list. This happens because we're using defaultValue.
        size_t currentIndex;

    public:
        static Scheduler* The();

        void RegisterThread(Thread* thread);

        void Init();
        void SelectNext();
        void Yield();

        Thread* GetExecutingThread();
    };
}