#pragma once

#include <stdint-gcc.h>

#define PORT_PIT_CHANNEL_0 0x40
#define PORT_PIT_CHANNEL_1 0x41
#define PORT_PIT_CHANNEL_2 0x42
#define PORT_PIT_COMMAND 0x43
//channel that channel 0 sets high when terminal count is reached
#define PIT_DEFAULT_IRQ 0
#define PIT_FREQ_HZ 1193182

namespace Kernel::Drivers
{
    class PIT
    {
    private:

    public:
        static uint64_t ticks;

        static void Init();
        static void Disable();
    };
}