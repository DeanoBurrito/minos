#include <multiprocessing/Thread.h>
#include <drivers/APIC.h>

namespace Kernel::Multiprocessing
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

        static KernelThreadData* Init()
        { 
            return new KernelThreadData(); 
        }

        KernelThreadData()
        {
            ss = rsp = rflags = cs = rip = 0;
            rax = rbx = rcx = rdx = rsi = rdi = rbp = 0;
            r8 = r9 = r10 = r11 = r12= r13 = r14 = r15 = 0;
            ds = es = fs = gs = 0;
        }
    };

    void InitKernelThreadData(KernelThreadData** data)
    {
        *data = new KernelThreadData(); 
    }

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
        data->rflags = flags;

        //NOTE: setting all other segments to data segment (read/write, no execute) by default
        data->ds = data->es = data->ss = data->fs = data->gs = dataSegment;
    }
}