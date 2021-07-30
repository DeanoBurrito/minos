#include <Panic.h>
#include <KRenderer.h>
#include <CPU.h>

namespace Kernel
{
    void Panic(const char* reason)
    {
        KRenderer::The()->Clear(Color(0xC0000000)); //red background
        KRenderer::The()->SetCursor(0, 0);

        KRenderer::The()->WriteLine((const char*)"Oops! Kernel panic! :(");
        KRenderer::The()->WriteLine((const char*)"");
        KRenderer::The()->WriteLine((const char*)"Reason:");
        KRenderer::The()->Write((const char*)"    ");
        KRenderer::The()->WriteLine(reason);

        CPU::Halt();
    }
}