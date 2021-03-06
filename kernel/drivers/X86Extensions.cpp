#include <drivers/X86Extensions.h>
#include <drivers/CPU.h>
#include <Platform.h>
#include <KLog.h>
#include <String.h>
#include <Formatting.h>
#include <cpuid.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64);

#define FXSAVE_BUFFER_SIZE 512
#define XSAVE_BUFFER_BASE 576

namespace Kernel::Drivers
{
    X86Extensions x86ExtsInstance;
    X86Extensions* X86Extensions::Local()
    {
        return &x86ExtsInstance;
    }

    void X86Extensions::Init()
    {   
        //check what extensions are available, then enable them.
        //NOTE: x86_64 spec requires at least sse2, but I like to check anyway
        size_t highestFeature = 0;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSE))
            highestFeature = 10;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSE2))
            highestFeature = 20;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSE3))
            highestFeature = 30;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSSE3))
            highestFeature = 31;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSE4_1))
            highestFeature = 41;
        if (CPU::FeatureSupported(CpuFeatureFlag::SSE4_2))
            highestFeature = 42;
        
        if (CPU::FeatureSupported(CpuFeatureFlag::AVX))
            highestFeature = 50;

        xSaveSupport = CPU::FeatureSupported(CpuFeatureFlag::XSAVE) && ALLOW_XSAVE;

        //enforce spec restrictions, ensuring cpuid has worked correctly.
        if (highestFeature >= 50 && !xSaveSupport)
        {
            Log("X86 extensions error: AVX supported, but not XSAVE. Cannot continue.");
            return;
        }
        if (highestFeature < 20)
        {
            Log("X86 extensions error: long mode requires SSE2 or greater support, but cpuid says this is unavailable. Cannot continue.");
            return;
        }
        if (!CPU::FeatureSupported(CpuFeatureFlag::FXSR) && !xSaveSupport)
        {
            Log("X86 extensions error: neither XSAVE or FXSR features are available. Cannot continue.");
            return;
        }
        if (!xSaveSupport)
            Log("XSAVE extension is not supported, defaulting to FXSAVE instead.");
            
        //info gathered, now we setup the CPU.
        SetStateHandling(false); //use pro-active state handling by default

        //cr0 setup
        {
            //setup cr0: read in existing value, forcibly clear bit 2 (EMulated), and apply flags we want
            uint64_t cr0Flags = 0;
            asm volatile("mov %%cr0, %0" : "=r"(cr0Flags));

            cr0Flags = cr0Flags & (uint64_t)~(1 << 2); //clear EM (bit2), emulation of fpu not allowed for sse/avx
            cr0Flags |= 1 << 4; //use 387+ protocol (hardwired, but still)
            cr0Flags |= 1 << 5; //NE: use native exceptions
            cr0Flags |= 1 << 1; //MP: monitor processor, required for SSE
            if (lazyStateHandling)
                cr0Flags |= 1 << 3; //TS: #NM device not found exception through when accessing extended state

            asm volatile("mov %0, %%cr0" :: "r"(cr0Flags));
        }
        
        //cr4 setup
        {
            //setup cr4 (confirm fxsave use and #XM exception handlers)
            uint64_t cr4Flags = 0;
            asm volatile("mov %%cr4, %0" : "=r"(cr4Flags)); //no currently necessary (or works fine), just for consistency with cr0 ops.

            cr4Flags |= 1 << 9; //OSFDSR - confirm os supports fxsave/fxrestore
            cr4Flags |= 1 << 10; //OSXMMEXCPT - confirm os supports #XM (sse exceptions)
            if (xSaveSupport)
                cr4Flags |= 1 << 18; //enable XSAVE support

            asm volatile("mov %0, %%cr4":: "r"(cr4Flags));
        }

        //fpu setup
        asm volatile("finit");
        
        //if available, get the components supported by xsave
        if (xSaveSupport)
        {
            uint32_t eax, ebx, ecx, edx; //32bit regs because of spec
            uint32_t cpuidLeaf = X86_CPUID_XSAVE;
            uint32_t cpuidSubleaf = 0;

            if (!__get_cpuid_count(cpuidLeaf, cpuidSubleaf, &eax, &ebx, &ecx, &edx))
            {
                Log("X86 Extensions error: Could not get XSAVE bitmap, even though it is supported. Cannot continue.");
                return;
            }
            xSaveAllowed = (uint64_t)edx << 32 | eax;
            
            //set xcr0 with default values
            xSaveMask = 1; //always save x87 FPU
            if (highestFeature > 0)
                xSaveMask |= 1 << 1; //save SSE state (XMM)
            if (highestFeature > 50)
                xSaveMask |= 1 << 2; //save avx state (YMM)
            
            xSaveMask = xSaveMask & xSaveAllowed; //we'll #GP if we try to set a value in xcr0 that is not allowed
            asm volatile("xsetbv" :: "a"(xSaveMask), "d"(xSaveMask >> 32), "c"(0));

            string fstr = "XSAVE allowed=0x%llx, selected=0x%llx";
            Log(sl::FormatToString(0, &fstr, xSaveAllowed, xSaveMask).Data());

            fstr = "XSAVE can require upto %u bytes for maximum feature set, we will use %u bytes.";
            Log(sl::FormatToString(0, &fstr, ecx, ebx).Data());
        }
        else
        {
            xSaveAllowed = 0;
            xSaveMask = 0;
        }

        string fstr = "X86 extensions initialized, featureLevel=%llu, xSaveSupport=%b, lazyManagement=%b";
        Log(sl::FormatToString(0, &fstr, highestFeature, xSaveSupport, lazyStateHandling).Data());
    }

    void X86Extensions::SetStateHandling(bool useLazy)
    {
        lazyStateHandling = useLazy;
    }

    size_t X86Extensions::GetStateBufferSize()
    {
        if (xSaveSupport)
            return 0;
        
        return FXSAVE_BUFFER_SIZE;
    }

    void X86Extensions::SaveState(void* dest)
    {
        if (xSaveSupport)
            asm volatile("xsave (%0)" :: "r"(dest), "a"(xSaveMask), "d"(xSaveMask >> 32) : "memory");
        else
            asm volatile("fxsave (%0)" :: "r"(dest) : "memory");
    }

    void X86Extensions::LoadState(void* source)
    {
        if (xSaveSupport)
            asm volatile("xrstor (%0)" :: "r"(source), "a"(xSaveMask), "d"(xSaveMask >> 32) : "memory");
        else
            asm volatile("fxrstor (%0)" :: "r"(source));
    }
}
