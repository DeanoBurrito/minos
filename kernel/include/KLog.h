#pragma once

#define LOGGING_DEFAULT_RESERVE_COUNT 4096

namespace Kernel
{
    void InitLogging();
    void SetSerialLogging(bool enabled);
    void Log(const char* message, bool endLine = true);
    void LogError(const char* message, bool endLine = true);

    unsigned long LogsUnread();
    //NOTE: will populate the buffer with char* of all unread log messages. It is expected to be of the correct length.
    void ReceiveLogs(const char** logsBuffer, unsigned long* count);
}
