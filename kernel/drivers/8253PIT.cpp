#include <drivers/8253PIT.h>
#include <drivers/CPU.h>

namespace Kernel::Drivers
{
    void PIT::Init()
    {
        //Fixed 1MHz-ish timer
        //uint16_t divisor = PIT_FREQ_HZ / hertz; 
        CPU::PortWrite8(PORT_PIT_COMMAND, 0x34);
        CPU::PortWrite8(PORT_PIT_CHANNEL_0, 0xA9);
        CPU::PortWrite8(PORT_PIT_CHANNEL_0, 0x04);
    }
}