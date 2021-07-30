#pragma once

#include <stdint-gcc.h>

#define PORT_COM1_ADDRESS 0x03F8

namespace Kernel
{
    class SerialPort
    {
        enum class RegisterOffsets : uint16_t
        {
            Data = 0,
            InterruptEnable = 1,
            InterruptIdentity_FifoControl = 2,
            LineControl = 3,
            ModemControl = 4,
            LineStatus = 5,
            ModemStatus = 6,
            ScratchRegister = 7,
        };

        enum class LineStatus : uint8_t
        {
            DataReady = 0,
            OverrunError = 1,
            ParityError = 2,
            FramingError = 3,
            BreakIndicator = 4,
            TransmitterBufferEmpty = 5,
            TransmitterIdle = 6,
            ImpendingError = 7,
        };

    private:
        bool initialized = false;
        uint16_t address;

    public:
        static SerialPort* COM1();

        bool Init(uint16_t portAddr);

        uint8_t ReadByte();
        void WriteByte(uint8_t data);

        bool ReadyToSend();
        bool ReadyToReceive();

        bool HasLineFlag(SerialPort::LineStatus flag);
    };

}
