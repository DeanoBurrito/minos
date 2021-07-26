#pragma once

#include <drivers/KeyboardCodes.h>

//Note: this is working off US/english keyboard layouts, may need to lengthen this in future.
#define PS2_PACKET_LENGTH 4
#define INVALID_PRINTABLE_CHAR_LITERAL '?'

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
    
    private:
        bool ProcessPacket();
    };
}