#pragma once

#include <stddef.h>
#include <stdint.h>

//useful for debugging fxsave stuff, otherwise leave enabled.
#define ALLOW_XSAVE true

namespace Kernel::Drivers
{
    /* 
        General wrapper for X86 cpu extensions: X87 floating point unit, SSE and AVX.
        NOTE: These operations apply only on the local processor.
    */

    class X86Extensions
    {
    private:
        bool lazyStateHandling; //using cpu exceptions to save/restore state
        bool xSaveSupport;
        uint64_t xSaveAllowed;
        uint64_t xSaveMask;

    public:
        static X86Extensions* Local();
        
        void Init();
        void SetStateHandling(bool useLazy);

        //returns size of buffer required to hold extended CPU state snapshot
        size_t GetStateBufferSize();

        //save current x86 extended state to buffer. NOTE: buffer is assumed big enough for state.
        void SaveState(void* dest);
        //load x86 extended state from buffer.
        void LoadState(void* source);
    };
}
