#include <Panic.h>
#include <KRenderer.h>
#include <drivers/CPU.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

/*  Panic subsystem design notes:
    -Panic needs to be completely self sufficient, and should work assuming all other memory and structures are corrupted.
        -However, I dont see any reason for now allowing it have some init code (if its all very simple logic).
    -Since we're in long mode, paging is required, and we'll need a known-good paging table.
    -We'll also need a working framebuffer.
        -Seeing as GOP is always available, we can make a copy of its initial values, even if a separate framebuffer is used later.
        -We may need a way to force any display stuff to use the gop framebuffer later on, as i'm not putting device drivers here.
    -Panic will also need it's own internal heap, separate from the rest of the system's allocators.
    -Thinking about how panic will be called, i'm thinking of mapping the call to a naked function that issues an interrupt.
        -With the interrupt handler being the function's body, this will limit what we can do further, but also limit system contamination.
        -The alternative is trying to extract a few franes anyway, and hoping we dont do anymore damage.

    Implementation ideas:
    -After memory is initialized, we make a copy of the kernel's identity mapped table.
    -Panic also claims a section of memory, getting it marked reserved (separate to used). Alternatively the bootloader could do this, and then
        we bypass pmm/vmm altogether. It becomes like any other reserved memory.
    -Grab copies of any other data we need (gop framebuffer details).
    -Set aside a region of memory for the panic message - this needs to be statically allocated, so its always available.

    -I like the idea of having a 'header' or sorts surrounded the panic code, with magic values we can check.
    -If this memory is overwritten at some point, we can jump to an alternate panic module (we init it twice, just with different base addresses).
    -This copy should be way off in memory somewhere, if its also corrupted, then oh well. Bigger issues.
*/

namespace Kernel
{
    void Panic(const char* reason)
    {
        //reset display with red background
        KRenderer::The()->Clear(Colour(0xC0000000));
        KRenderer::The()->SetCursor({0, 1});

        //emit general description and error message
        KRenderer::The()->WriteLine("Oops! Kernel panic! :(");
        KRenderer::The()->WriteLine("");
        KRenderer::The()->WriteLine("Reason:");
        KRenderer::The()->Write("    ");
        KRenderer::The()->WriteLine(reason);

        //TODO: capture register state (of previous frame) and display it, as well as useful memory/dev info

        Drivers::CPU::Halt();
    }
}