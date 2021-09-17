#include <Panic.h>
#include <KRenderer.h>
#include <drivers/CPU.h>
#include <Platform.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

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