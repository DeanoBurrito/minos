#pragma once

#define LOGGING_DEFAULT_RESERVE_COUNT 4096
#define PORT_DEBUGCON_ADDRESS 0xE9

namespace Kernel
{
    enum LoggingType
    {
        //live big and write directly to the framebuffer. Use with caution.
        FramebufferDirect,
        //emit log messages over COM1
        Serial,
        //emit log messages over bochs debugcon, check for support first.
        DebugCon,

        Count
    };
    
    void InitLogging();
    void SetLogTypeEnabled(LoggingType type, bool enabled);

    void Log(const char* message, bool endLine = true);
    void LogError(const char* message, bool endLine = true);

    unsigned long LogsUnread();
    //NOTE: will populate the buffer with char* of all unread log messages. It is expected to be of the correct length.
    void ReceiveLogs(const char** logsBuffer, unsigned long* count);
}
