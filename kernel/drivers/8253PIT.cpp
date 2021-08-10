#include <drivers/8253PIT.h>
#include <drivers/CPU.h>

namespace Kernel::Drivers
{
    void PIT::StartTimer(uint16_t hertz)
    {
        uint16_t divisor = PIT_FREQ_HZ / hertz; 
        CPU::PortWrite8(PORT_PIT_COMMAND, 0x36);
        CPU::PortWrite8(PORT_PIT_CHANNEL_0, divisor & 0xFF);
        CPU::PortWrite8(PORT_PIT_CHANNEL_0, (divisor & 0xFF00) >> 8);
    }

    void PIT::Disable()
    {
        //TODO: 
    }
}