#include <drivers/KeyboardCodes.h>

namespace Kernel::Drivers
{
    KeyModifierFlags operator&(const KeyModifierFlags& l, const KeyModifierFlags& r)
    {
        return (KeyModifierFlags)((uint32_t)l & (uint32_t)r);
    }

    KeyModifierFlags operator|(const KeyModifierFlags& l, const KeyModifierFlags& r)
    {
        return (KeyModifierFlags)((uint32_t)l | (uint32_t)r);
    }

    KeyModifierFlags operator~(const KeyModifierFlags& operand)
    {
        return (KeyModifierFlags)~((uint32_t)operand); //the critical wiggle :D
    }

    KeyModifierFlags KeyModifierFromKey(const KeyboardKey& key)
    {
        switch (key)
        {   //TODO: do we really need this? static data like this would be better an array of fixed data, rather than code.
        case KeyboardKey::LeftShift:
            return KeyModifierFlags::LeftShift;
        case KeyboardKey::RightShift:
            return KeyModifierFlags::RightShift;
        case KeyboardKey::LeftCtrl:
            return KeyModifierFlags::LeftCtrl;
        case KeyboardKey::RightCtrl:
            return KeyModifierFlags::RightCtrl;
        case KeyboardKey::LeftAlt:
            return KeyModifierFlags::LeftAlt;
        case KeyboardKey::RightAlt:
            return KeyModifierFlags::RightAlt;
        case KeyboardKey::LeftGui:
            return KeyModifierFlags::LeftGui;
        case KeyboardKey::RightGui:
            return KeyModifierFlags::RightGui;
        default:
            return KeyModifierFlags::None;
        }
    }

    bool KeyIsModifier(const KeyboardKey& key)
    {
        return KeyModifierFromKey(key) != KeyModifierFlags::None;
    }
    
    namespace US_QWERTY
    {
        KeyboardKey set1Map[] 
        {
            KeyboardKey::InvalidKey, KeyboardKey::Escape, KeyboardKey::Num1, KeyboardKey::Num2, 
            KeyboardKey::Num3, KeyboardKey::Num4, KeyboardKey::Num5, KeyboardKey::Num6, 
            KeyboardKey::Num7, KeyboardKey::Num8, KeyboardKey::Num9, KeyboardKey::Num0, 
            KeyboardKey::Minus, KeyboardKey::Equals, KeyboardKey::Backspace, KeyboardKey::Tab,
            KeyboardKey::Q, KeyboardKey::W, KeyboardKey::E, KeyboardKey::R,
            KeyboardKey::T, KeyboardKey::Y, KeyboardKey::U, KeyboardKey::I,
            KeyboardKey::O, KeyboardKey::P, KeyboardKey::LeftSquareBracket, KeyboardKey::RightSquareBracket,
            KeyboardKey::Enter, KeyboardKey::LeftCtrl, KeyboardKey::A, KeyboardKey::S,
            KeyboardKey::D, KeyboardKey::F, KeyboardKey::G, KeyboardKey::H,
            KeyboardKey::J, KeyboardKey::K, KeyboardKey::L, KeyboardKey::Semicolon,
            KeyboardKey::SingleQuote, KeyboardKey::Backtick, KeyboardKey::LeftShift, KeyboardKey::BackSlash,
            KeyboardKey::Z, KeyboardKey::X, KeyboardKey::C, KeyboardKey::V,
            KeyboardKey::B, KeyboardKey::N, KeyboardKey::M, KeyboardKey::Comma,
            KeyboardKey::Fullstop, KeyboardKey::ForwardSlash, KeyboardKey::RightShift, KeyboardKey::NumpadStar,
            KeyboardKey::LeftAlt, KeyboardKey::Space, KeyboardKey::CapsLock, KeyboardKey::F1,
            KeyboardKey::F2, KeyboardKey::F3, KeyboardKey::F4, KeyboardKey::F5,
            KeyboardKey::F6, KeyboardKey::F7, KeyboardKey::F8, KeyboardKey::F9,
            KeyboardKey::F10, KeyboardKey::NumLock, KeyboardKey::ScrollLock, KeyboardKey::Numpad7,
            KeyboardKey::Numpad7, KeyboardKey::Numpad7, KeyboardKey::NumpadMinus, KeyboardKey::Numpad4,
            KeyboardKey::Numpad5, KeyboardKey::Numpad5, KeyboardKey::NumpadPlus, KeyboardKey::Numpad1,
            KeyboardKey::Numpad1, KeyboardKey::Numpad3, KeyboardKey::Numpad0, KeyboardKey::NumpadFullstop,
            KeyboardKey::InvalidKey, KeyboardKey::InvalidKey, KeyboardKey::InvalidKey, KeyboardKey::F11,
            KeyboardKey::F12
        };

        KeyAction Translate(uint8_t* packet, int packetLength)
        {
            //TODO: assuming we're using Scancode set 1 here
            if (packetLength > 1)
                return KeyAction();
            
            if (packet[0] > 0x58)
            {
                //it's a release
                packet[0] = packet[0] - 0x80;
                
                KeyAction result;
                result.key = set1Map[packet[0]];
                result.flags = KeyModifierFlags::Released;
                return result;
            }
            else
            {
                KeyAction result;
                result.key = set1Map[packet[0]];
                result.flags = KeyModifierFlags::Pressed;
                return result;
            }

            return KeyAction();
        }
    }

    namespace Printable
    {
        const char digitsLower[]
        {
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
        };

        const char digitsUpper[]
        {
            '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'
        };

        const char lettersLower[]
        {
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
        };

        const char lettersUpper[]
        {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
        };

        const char otherLower[]
        {
            '[', ']', ';', '\'', '`', '\\', ',', '.', '/', '-', ' ', '='
        };

        const char otherUpper[]
        {
            '{', '}', ':', '"', '~', '|', '<', '>', '?', '_', ' ', '+'
        };
    }
}