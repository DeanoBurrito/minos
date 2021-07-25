#pragma once

namespace Kernel
{
    void SetSerialLogging(bool enabled);
    void SetRenderedLogging(bool enabled);
    void Log(const char* message, bool endLine = true);
    void LogError(const char* message, bool endLine = true);
}
