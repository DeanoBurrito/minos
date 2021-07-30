#include <drivers/Ps2Keyboard.h>
#include <KLog.h>
#include <CPU.h>

namespace Kernel::Drivers
{
    Ps2Keyboard instance;
    Ps2Keyboard* Ps2Keyboard::The()
    {
        return &instance;
    }
    
    uint8_t packetBytes[PS2_PACKET_LENGTH];
    int packetLength = 0;
    void Ps2Keyboard::HandlePacketByte(uint8_t packetByte)
    {
        //NOTE: this is called from inside an interrupt, be careful.
        if (packetLength + 1 > PS2_PACKET_LENGTH)
        {
            LogError("PS2 Keyboard packet buffer is full, but yet to be processed.");
            return; //drop the packet
        }

        packetBytes[packetLength] = packetByte;

        if (ProcessPacket()) //reset packet if we successfully read from it
            packetLength = 0;
    }

    ScancodeSet currentSet = ScancodeSet::Set1;
    KeyModifierFlags currentModifiers = KeyModifierFlags::None;
    bool Ps2Keyboard::ProcessPacket()
    {
        if (currentSet == ScancodeSet::Set1)
        {
            if (packetLength == 1 && packetBytes[0] == 0xE0) //sets 1 & 2 extension code, we need another byte to determine key
                return false;
            
            KeyAction action = US_QWERTY::Translate(packetBytes, packetLength);

            //handle modifiers
            if (KeyIsModifier(action.key))
            {
                if ((action.flags & KeyModifierFlags::Pressed) == KeyModifierFlags::Pressed)
                {
                    currentModifiers = currentModifiers | KeyModifierFromKey(action.key);
                }
                else if ((action.flags & KeyModifierFlags::Released) == KeyModifierFlags::Released)
                {
                    currentModifiers = currentModifiers & ~KeyModifierFromKey(action.key);
                }
                else
                { /* not sure what happened, if it wasnt pressed or released. TODO: emit error */ }
            }

            if ((action.flags & KeyModifierFlags::Pressed) == KeyModifierFlags::Pressed && IsPrintable(action.key))
            {
                char outbuf[] { GetPrintable(action.key, (currentModifiers & KeyModifierFlags::LeftShift) == KeyModifierFlags::LeftShift), 0 };
                Log(outbuf, false);
            }

            //TODO: store these somewhere, for later consumption
            return true;
        }
        else if (currentSet == ScancodeSet::Set2)
        {
        }

        //TODO: error handling with invalid set
        return true;
    }
    
    bool Ps2Keyboard::IsPrintable(KeyboardKey key)
    { 
        return (key >= KeyboardKey::Num1 && key <= KeyboardKey::Num0) 
            || (key >= KeyboardKey::A && key <= KeyboardKey::Z)
            || (key >= KeyboardKey::LeftSquareBracket && key <= KeyboardKey::Equals);
    }

    char Ps2Keyboard::GetPrintable(KeyboardKey key, bool shifted)
    { 
        if (key >= KeyboardKey::Num1 && key <= KeyboardKey::Num0)
        {
            int index = (int)key - (int)KeyboardKey::Num1;
            return shifted ? Printable::digitsUpper[index] : Printable::digitsLower[index];
        }
        else if (key >= KeyboardKey::A && key <= KeyboardKey::Z)
        {
            int index = (int)key - (int)KeyboardKey::A;
            return shifted ? Printable::lettersUpper[index] : Printable::lettersLower[index];
        }
        else if (key >= KeyboardKey::LeftSquareBracket && key <= KeyboardKey::Equals)
        {
            int index = (int)key - (int)KeyboardKey::LeftSquareBracket;
            return shifted ? Printable::otherUpper[index] : Printable::otherLower[index];
        }
        else
            return INVALID_PRINTABLE_CHAR_LITERAL;
    }

    ScancodeSet Ps2Keyboard::GetScancodeSet()
    {
        return currentSet;
    }

    void Ps2Keyboard::SetKeyRepeat(uint8_t repeatRate, uint8_t delay)
    {
        uint8_t packed = (repeatRate & 0b0001'1111) | ((delay & 0b11) << 5);

        bool interruptsEnabled = CPU::InterruptsEnabled();
        CPU::DisableInterrupts();

        //send command byte, then data, ensure we get an ACK response
        CPU::PortWrite8(PORT_PS2_KEYBOARD, 0xF3); //0xF3 = set typematic rate and delay command
        CPU::PortIOWait();
        CPU::PortWrite8(PORT_PS2_KEYBOARD, packed);
        CPU::PortIOWait();
        uint8_t response = CPU::PortRead8(PORT_PS2_KEYBOARD);
        if (interruptsEnabled)
            CPU::EnableInterrupts();

        if (response != 0xFA) //0xFA = ACK
            LogError("PS2 keyboard was not happy with key repeat values sent.");
    }
}