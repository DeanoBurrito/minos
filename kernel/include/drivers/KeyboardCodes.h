#pragma once

#include <stdint-gcc.h>

namespace Kernel::Drivers
{
    enum class KeyboardKey : uint32_t
    {
        InvalidKey = 0x0,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Num0,

        //Modifiers
        LeftCtrl,
        RightCtrl,
        LeftAlt,
        RightAlt,
        LeftShift,
        RightShift,
        LeftGui,
        RightGui,

        //Text control keys
        Escape,
        Backspace,
        Tab,
        Enter,
        Home,
        End,
        Delete,
        Insert,

        //Non-letter printable
        LeftSquareBracket,
        RightSquareBracket,
        Semicolon,
        SingleQuote,
        Backtick,
        BackSlash,
        Comma,
        Fullstop,
        ForwardSlash,
        Minus,
        Space,
        Equals,
        
        //non-printable
        ArrowUp,
        ArrowDown,
        ArrowLeft,
        ArrowRight,
        PageUp,
        PageDown,
        PrintScreen,
        Pause,

        //Stateful keys
        CapsLock,
        NumLock,
        ScrollLock,

        //Letters
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        
        //Numpad/Keypad stuff
        Numpad7,
        Numpad8,
        Numpad9,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad0,
        NumpadMinus,
        NumpadPlus,
        NumpadStar,
        NumpadFullstop,
    };

    enum class KeyModifierFlags : uint32_t
    {
        None = 0,
        All = 0xffffffff,

        Pressed = 1 << 0,
        Released = 1 << 1,

        LeftShift = 1 << 2,
        RightShift = 1 << 3,
        LeftCtrl = 1 << 4,
        RightCtrl = 1 << 5,
        LeftAlt = 1 << 6,
        RightAlt = 1 << 7,
        LeftGui = 1 << 8,
        RightGui = 1 << 9,

        AnyShift = 3 << 2,
        AnyCtrl = 3 << 4,
        AnyAlt = 3 << 6,
        AnyGui = 3 << 8,
    };

    KeyModifierFlags operator&(const KeyModifierFlags& l, const KeyModifierFlags& r);
    KeyModifierFlags operator|(const KeyModifierFlags& l, const KeyModifierFlags& r);
    KeyModifierFlags operator~(const KeyModifierFlags& operand);
    
    KeyModifierFlags KeyModifierFromKey(KeyboardKey& key);
    bool KeyIsModifier(KeyboardKey& key);

    class KeyAction
    {
    public:
        KeyboardKey key;
        KeyModifierFlags flags = KeyModifierFlags::None;

        KeyAction() : key(KeyboardKey::InvalidKey) {}
        KeyAction(KeyboardKey key) : key(key) {}
    };
    
    namespace US_QWERTY
    {
        KeyAction Translate(uint8_t* packet, int packetLength);
    }

    namespace Printable
    {
        extern const char digitsLower[];
        extern const char digitsUpper[];
        extern const char lettersLower[];
        extern const char lettersUpper[];
        extern const char otherLower[];
        extern const char otherUpper[];
    }
}