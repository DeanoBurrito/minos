#pragma once

#include <drivers/KeyboardCodes.h>

//Note: this is working off US/english keyboard layouts, may need to lengthen this in future.
#define PS2_PACKET_LENGTH 4
#define INVALID_PRINTABLE_CHAR_LITERAL '?'
#define PORT_PS2_KEYBOARD 0x60

namespace Kernel::Drivers
{
    enum class ScancodeSet
    {
        Set1,
        Set2,
    };
    
    class Ps2Keyboard
    {
    public:
        static Ps2Keyboard* The();
        void HandlePacketByte(uint8_t packetByte);
        
        bool IsPrintable(KeyboardKey key);
        char GetPrintable(KeyboardKey key, bool shifted);
        ScancodeSet GetScancodeSet();

        //check the ps/2 specs for these. RepeatRate is 5 bits, higher is slower. Delay is 2 bits, 250ms per +0b1
        void SetKeyRepeat(uint8_t repeatRate, uint8_t delay);
    
    private:
        bool ProcessPacket();
    };
}