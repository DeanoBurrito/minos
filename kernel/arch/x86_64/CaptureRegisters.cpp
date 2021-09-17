#include <arch/x86_64/CaptureRegisters.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

extern "C"
{
    Kernel::CapturedRegisters capturedRegistersBuffer;
    void CaptureRegisters_impl();
}

namespace Kernel
{
    void CaptureRegisters(CapturedRegisters* stash)
    {
        //no need for memset here, existing buffer data will be completely overwritten.
        CaptureRegisters_impl();
        *stash = capturedRegistersBuffer;
    }
}
