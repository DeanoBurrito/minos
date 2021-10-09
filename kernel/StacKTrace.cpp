#include <StackTrace.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64);

namespace Kernel
{
    StackTrace GetCurrentTrace(size_t maxDepth)
    {
        if (maxDepth == 0)
            maxDepth = UINT32_UPPER_LIMIT;

        StackTrace trace;
        NativePtr frameBase = 0;
        NativePtr prevInstructionPointer = 0;

        asm volatile("mov %%rbp, %0" : "=r"(frameBase));

        while (maxDepth > 0 && frameBase > 0)
        {
            //NOTE: this is specific to system v x86_64 calling convention
            asm volatile("mov 8(%1), %0" : "=r"(prevInstructionPointer) : "r"(frameBase) : "memory");
            asm volatile("mov 0(%1), %0" : "=r"(frameBase) : "r"(frameBase) : "memory");

            maxDepth--;
            trace.frames.PushBack(prevInstructionPointer);
        }

        return trace;
    }
}
