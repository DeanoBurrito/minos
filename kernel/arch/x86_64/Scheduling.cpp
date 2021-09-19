#include <multiprocessing/Thread.h>
#include <multiprocessing/Scheduler.h>

namespace Kernel::Multiprocessing
{
    void Thread::ThreadArchInit(Thread* thread, uint64_t entryAddr, bool hasKernelPriv)
    {
        /*
            On x86_64 we're storing: ss, rsp, rflags, cs, rip (all things we need to create an iret frame)
        */
        //TODO: prime stack with initial argument, and then all blanked registers so we can pop them later.

        thread->implementationData[0] = 0x10;
        thread->implementationData[1] = (uint64_t)thread->stackMax;
        thread->implementationData[2] = 0x202;
        thread->implementationData[3] = 0x8;
        thread->implementationData[4] = entryAddr;
    }
}
