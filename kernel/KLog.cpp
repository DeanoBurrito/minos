#include "Serial.h"
#include "KRenderer.h"
#include "KLog.h"

namespace Kernel
{
    bool serialLogEnabled = false;
    void SetSerialLogging(bool enabled)
    {
        serialLogEnabled = enabled;
    }

    bool renderedLogEnabled = false;
    void SetRenderedLogging(bool enabled)
    {
        renderedLogEnabled = enabled;
    }

    void Log(const char* message, bool endLine)
    {
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

        if (renderedLogEnabled)
            KRenderer::The()->WriteLine(message);
    }

    void LogError(const char* message, bool endLine)
    {
        Log(message, endLine); //TODO: implement endpoint specific stuff to highlight error
    }
}
