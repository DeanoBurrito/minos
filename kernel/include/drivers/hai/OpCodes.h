#pragma once

#include <stdint.h>

namespace Kernel::Drivers::Hai
{
    //Core AML op codes
    enum AmlOp : uint8_t
    {
        Zero = 0x00,
        One = 0x01,

        Alias = 0x06,
        Name = 0x08,
        
        BytePrefix = 0x0A,
        WordPrefix = 0x0B,
        DoubleWordPrefix = 0x0C,
        StringPrefix = 0x0D,
        QuadWordPrefix = 0x0E,

        Scope = 0x10,
        Buffer = 0x11,
        Package = 0x12,
        VarPackage = 0x13,
        Method = 0x14,
        External = 0x15,

        DualNamePrefix = 0x2E,
        MultiNamePrefix = 0x2F,
        
        DigitChar_0 = 0x30,
        DigitChar_1 = 0x31,
        DigitChar_2 = 0x32,
        DigitChar_3 = 0x33,
        DigitChar_4 = 0x34,
        DigitChar_5 = 0x35,
        DigitChar_6 = 0x36,
        DigitChar_7 = 0x37,
        DigitChar_8 = 0x38,
        DigitChar_9 = 0x39,

        //I cant believe I wrote this by hand - I impress myself
        NameChar_A = 0x41,
        NameChar_B = 0x42,
        NameChar_C = 0x43,
        NameChar_D = 0x44,
        NameChar_E = 0x45,
        NameChar_F = 0x46,
        NameChar_G = 0x47,
        NameChar_H = 0x48,
        NameChar_I = 0x49,
        NameChar_J = 0x4A,
        NameChar_K = 0x4B,
        NameChar_L = 0x4C,
        NameChar_M = 0x4D,
        NameChar_N = 0x4E,
        NameChar_O = 0x4F,
        NameChar_P = 0x50,
        NameChar_Q = 0x51,
        NameChar_R = 0x52,
        NameChar_S = 0x53,
        NameChar_T = 0x54,
        NameChar_U = 0x55,
        NameChar_V = 0x56,
        NameChar_W = 0x57,
        NameChar_X = 0x58,
        NameChar_Y = 0x59,
        NameChar_Z = 0x5A,

        //go straight to table 2, do not pass go, do not collect 200+ performance
        ExtOpPrefix = 0x5B,

        RootChar = 0x5C,
        ParentPrefix = 0x5E,
        NameChar = 0x5F,

        Local_0 = 0x60,
        Local_1 = 0x61,
        Local_2 = 0x62,
        Local_3 = 0x63,
        Local_4 = 0x64,
        Local_5 = 0x65,
        Local_6 = 0x66,
        Local_7 = 0x67,

        Arg_0 = 0x68,
        Arg_1 = 0x69,
        Arg_2 = 0x6A,
        Arg_3 = 0x6B,
        Arg_4 = 0x6C,
        Arg_5 = 0x6D,
        Arg_6 = 0x6E,
        Arg_7 = 0x6F,

        Store = 0x70,
        RefOf = 0x71,

        Add = 0x72,
        Concat = 0x73,
        Subtract = 0x74,
        Increment = 0x75,
        Decrement = 0x76,
        Multiply = 0x77,
        Divide = 0x78,
        ShiftLeft = 0x79,
        ShiftRight = 0x7A,
        And = 0x7B,
        Nand = 0x7C,
        Or = 0x7D,
        Nor = 0x7E,
        Xor = 0x7F,
        Not = 0x80,

        FindSetLeftBit = 0x81,
        FindSetRightBit = 0x82,
        DerefOf = 0x83,
        ConcatRes = 0x84,
        Mod = 0x85,
        Notify = 0x86,
        SizeOf = 0x87,
        Index = 0x88,
        Match = 0x89,

        CreateDoubleWordField = 0x8A,
        CreateWordField = 0x8B,
        CreateByteField = 0x8C,
        CreateBitField = 0x8D,
        ObjectType = 0x8E,
        CreateQuadWordField = 0x8F,

        LogicalAZnd = 0x90,
        LogicalOr = 0x91,
        //NOTE: lookahead required, can be combined with 0x93-0x95
        LogicalNot = 0x92,
        //NOTE: combines to form !=
        LogicalEqual = 0x93,
        //NOTE: combines to form >=
        LogicalGreater = 0x94,
        //NOTE: combines to form <=
        LogicalLess = 0x95,
        
        ToBuffer = 0x96,
        ToDecimalString = 0x97,
        ToHexString = 0x98,
        ToIntegerString = 0x99,
        ToString = 0x9C,

        CopyObject = 0x9D,
        Mid = 0x9E,
        Continue = 0x9F,
        If = 0xA0,
        Else = 0xA1,
        While = 0xA2,
        Nop = 0xA3,
        Return = 0xA4,
        Break = 0xA5,
        
        Breakpoint = 0xCC,

        Ones = 0xFF
    };

    //Extended AML op codes
    enum AmlExtOp : uint8_t
    {
        Mutex = 0x01,
        Event = 0x02,

        ConditionalRef = 0x12,
        CreateField = 0x13,
        LoadTable = 0x1F,
        
        Load = 0x20,
        Stall = 0x21,
        Sleep = 0x22,
        Acquire = 0x23,
        Signal = 0x24,
        Wait = 0x25,
        Reset = 0x26,
        Release = 0x27,

        FromBCD = 0x28,
        ToBCD = 0x29,

        Revision = 0x30,
        Debug = 0x31,
        Fatal = 0x32,
        Timer = 0x33,

        OpRegion = 0x80,
        Field = 0x81,
        Device = 0x82,
        Processor = 0x83,
        PowerRes = 0x84,
        ThermalZone = 0x85,
        IndexField = 0x86,
        BankField = 0x87,
        DataRegion = 0x88,
    };
}
