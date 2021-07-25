#include "CPU.h"
#include "KLog.h"
#include "StringUtil.h"
#include "Serial.h"

#define PORT_TEST_BYTE 0xAE

namespace Kernel
{
    SerialPort com1;
    SerialPort* SerialPort::COM1()
    {
        return &com1;
    }
    
    bool SerialPort::Init(uint16_t portAddr)
    {
        if (initialized)
            return true;

        initialized = true;
        address = portAddr;
        Log("Initializing serial port at: 0x", false); Log(ToStrHex(address));

        //do the standard serial init dance
        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::InterruptEnable, 0x00);               //disable interrupts
        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::LineControl, 0x80);                   //enable DLAB, setting baud rate divisor
        CPU::PortWrite8(address + 0, 0x03);                                                        //setting divisor to 3 (38400 baud). low byte
        CPU::PortWrite8(address + 1, 0x00);                                                        //high byte
        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::LineControl, 0x03);                   //mode 3: 8 bits, no parity, one stop bit
        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::InterruptIdentity_FifoControl, 0xC7); //enable FIFO (14 byte threadshold before clear)
        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::ModemControl, 0x0B);                  //interrupts enabled, RTS/DSR set

        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::ModemControl, 0x1E); //set in loopback mode
        CPU::PortWrite8(address, PORT_TEST_BYTE);                                 //send ourselves a nice byte

        if (CPU::PortRead8(address) != PORT_TEST_BYTE)
        {
            Log("  \\- Serial might be faulty: loopback byte mismatch.");
            return false; //issue with loopback!
        }

        CPU::PortWrite8(address + (uint16_t)RegisterOffsets::ModemControl, 0x0F); //return to normal operation
        return true;
    }

    uint8_t SerialPort::ReadByte()
    {
        while (!ReadyToReceive());

        return CPU::PortRead8(address);
    }

    void SerialPort::WriteByte(uint8_t data)
    {
        while (!ReadyToSend());

        CPU::PortWrite8(address, data);
    }

    bool SerialPort::ReadyToSend()
    {
        return HasLineFlag(LineStatus::TransmitterBufferEmpty);
    }

    bool SerialPort::ReadyToReceive()
    {
        return HasLineFlag(LineStatus::DataReady);
    }

    bool SerialPort::HasLineFlag(SerialPort::LineStatus flag)
    {
        uint8_t mask = (1 << (uint8_t)flag);
        return (CPU::PortRead8(address + (uint16_t)RegisterOffsets::LineStatus) & mask) > 0;
    }
}
