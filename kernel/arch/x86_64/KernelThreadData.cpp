#include <multiprocessing/Thread.h>
#include <drivers/APIC.h>

namespace Kernel::Multipropcessing
{
    extern "C" void Asm_SendEOI()
    {
        Drivers::APIC::Local()->SendEOI();
    }
    
    //this is tightly integrated with Scheduler.asm, if this changes, change that too!
    struct KernelThreadData
    {
        //popped by iret
        uint64_t ss, rsp, rflags, cs, rip;

        //things we need to maintain
        uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
        uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
        uint64_t ds, es, fs, gs;
    };

    void SetKernelThreadEntry(KernelThreadData* data, uint64_t mainAddr, void* arg0, void* arg1)
    {
        data->rip = mainAddr;
        data->rdi = (uint64_t)arg0;
        data->rsi = (uint64_t)arg1;
    }

    void SetKernelThreadStack(KernelThreadData* data, uint64_t base)
    {
        data->rsp = data->rbp = base;
    }

    void SetKernelThreadFlags(KernelThreadData* data, uint64_t codeSegment, uint64_t dataSegment, uint64_t flags)
    {
        data->cs = codeSegment;
        data->ds = dataSegment;
        data->rflags = flags;
    }
}