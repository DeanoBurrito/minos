#include <drivers/Serial.h>
#include <KLog.h>
#include <collections/List.h>
#include <drivers/CPU.h>
#include <Memory.h>

namespace Kernel
{
    using namespace Kernel::Drivers;
    
    sl::List<const char*> unreadLogs; //TODO: have some failsafe for logs, as this can grow infinitely until it consumes all system memory.

    void InitLogging()
    {
        unreadLogs = sl::List<const char*>(LOGGING_DEFAULT_RESERVE_COUNT); //reserve some space, because we're going to use it for sure.
    }
    
    bool serialLogEnabled = false;
    void SetSerialLogging(bool enabled)
    {
        serialLogEnabled = enabled;
    }

    void Log(const char* message, bool endLine)
    {
        //unreadLogs.PushBack(msgCopy);
        
        if (serialLogEnabled)
        {
            for (int i = 0; message[i] != 0; i++)
            {
                SerialPort::COM1()->WriteByte(message[i]);
            }

            if (endLine)
            {
                //End line with CR LF
                SerialPort::COM1()->WriteByte('\n');
                SerialPort::COM1()->WriteByte('\r');
            }
        }
    }

    void LogError(const char* message, bool endLine)
    {
        Log(message, endLine); //TODO: implement endpoint specific stuff to highlight error
    }

    unsigned long LogsUnread()
    {
        return unreadLogs.Size();
    }

    void ReceiveLogs(const char** logsBuffer, unsigned long* count)
    {
        //since other threads might alter the logs, this cant be pre-empted
        bool interruptsEnabled = CPU::InterruptsEnabled();
        CPU::DisableInterrupts();

        unsigned long consumeCount = *count;
        if (consumeCount > unreadLogs.Size())
        {
            consumeCount = unreadLogs.Size();
            *count = consumeCount;
        }

        //since we only have PopBack(), we'll iterate backwards
        for (int i = consumeCount; i > 0; i--)
            logsBuffer[i - 1] = unreadLogs.PopBack();

        if (interruptsEnabled)
            CPU::EnableInterrupts();
    }
}
