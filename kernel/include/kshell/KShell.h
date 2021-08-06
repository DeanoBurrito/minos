#pragma once

#include <stdint-gcc.h>
#include <drivers/Ps2Keyboard.h>
#include <KRenderer.h>

#define KSHELL_INPUT_BUFFER_LEN 70
#define KSHELL_INCOMING_LOGS_MAX 32

#define KSHELL_OUTPUT_LIMIT_LINE displaySize.y - 4
#define KSHELL_PROMPT_LINE displaySize.y - 2
#define KSHELL_STATUS_LINE displaySize.y - 1
#define KSHELL_STATUS_BG_COLOUR Colour(0x303030FF)

namespace Kernel::Shell
{
    struct KShellPixel
    {
        uint8_t r, g, b, x;
    };
    
    class KShell
    {
    private:
        char inputLine[KSHELL_INPUT_BUFFER_LEN + 1];
        unsigned int inputLength;
        
        const char* incomingLogsBuffer[KSHELL_INCOMING_LOGS_MAX];

        Position displaySize;
        Position characterSize;
        Position writeCursorPos;
        uint32_t inputCursor;
        Colour bgColour;
        Colour fgColour;

        Drivers::KeyAction incomingKeysBuffer[KEYACTION_QUEUE_LENGTH];
        const char* promptText;
        uint8_t* dirtyLineLengths;

        bool keepRunning;
        bool echoSerialOut;

        void Init();
        void ProcessKey(const Drivers::KeyAction& key);
        void ProcessCommand();
        void ClearLine(uint32_t lineNum, uint32_t startCol, uint32_t endCol, Colour clearColour);

    public:
        static void ThreadMain(void* ignored);
        void Main();

        void WriteLine(const char* const line);
        void SetPrompt(const char* const text);
        void SetStatus(const char* const text);
    };
}