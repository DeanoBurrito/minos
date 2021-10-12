#include <kshell/KShell.h>
#include <multiprocessing/Thread.h>
#include <StringExtras.h>
#include <Memory.h>
#include <drivers/Serial.h>
#include <KLog.h>
#include <Panic.h>
#include <drivers/SystemClock.h>
#include <Limits.h>

using namespace Kernel::Drivers;

namespace Kernel::Shell
{
    void KShell::ThreadMain(void* ignored)
    {
        KShell instance;
        instance.Main();
    }

    void KShell::Init()
    {
        bgColour = 0x00'00'00'FF;
        fgColour = 0xFF'FF'FF'FF;
        inputCursor = inputLength = 0;
        sl::memset(inputLine, 0, KSHELL_INPUT_BUFFER_LEN + 1);
        promptText = "(kernel) - no FS $ ";
        echoSerialOut = true;

        sl::memset(incomingLogsBuffer, 0, KSHELL_INCOMING_LOGS_MAX);

        characterSize = KRenderer::The()->GetFontCharacterSize();
        displaySize = KRenderer::The()->GetFramebufferSize();
        displaySize = Position(displaySize.x / characterSize.x, displaySize.y / characterSize.y);
        KRenderer::The()->SetColours(fgColour, bgColour);
        KRenderer::The()->Clear();

        dirtyLineLengths = new uint8_t[displaySize.y];
        sl::memset(dirtyLineLengths, 0, displaySize.y);

        SetPrompt(nullptr);
        ClearLine(KSHELL_STATUS_LINE, 0, displaySize.x - 1, KSHELL_STATUS_BG_COLOUR);
        KRenderer::The()->SetCursor(Position(0, KSHELL_STATUS_LINE));
        KRenderer::The()->Write("STATUS: ");
        SetStatus(nullptr);
        writeCursorPos = Position(0, 0);

        cursorVisible = true;
        nextCursorToggleMS = SystemClock::The()->GetUptime() + KSHELL_CURSOR_BLINK_MS;
        statusClearTime = 0;

        WriteLine("Welcome to KShell! Nothing to do with KDE.");
    }

    void KShell::ProcessKey(const KeyAction& key)
    {
        if ((key.flags & KeyModifierFlags::Pressed) == KeyModifierFlags::None)
            return; //we only want key presses
        
        if (KeyIsModifier(key.key)) //we dont care about modifiers on their own, ignore them.
            return;

        if (Ps2Keyboard::The()->IsPrintable(key.key))
        {
            if (inputLength >= KSHELL_INPUT_BUFFER_LEN)
            {
                SetStatus("Reached input buffer limit!");
                return; //drop any keys if we're at the buffer limit
            }
            
            inputLine[inputLength] = Ps2Keyboard::The()->GetPrintable(key.key, (key.flags & KeyModifierFlags::AnyShift) != KeyModifierFlags::None);
            inputLength++;
            inputCursor++;
            SetPrompt(nullptr);
            return;
        }
        
        switch (key.key) 
        {
        case KeyboardKey::Backspace:
        {
            if (inputCursor > 0 && inputLength > 0)
            {
                sl::memcopy(inputLine + inputCursor, inputLine + inputCursor - 1, inputLength - inputCursor);
                inputLength--;
                inputCursor--;
                SetPrompt(nullptr);
            }
            break;
        }
        case KeyboardKey::Enter:
        {
            SetStatus("processing command ...");
            ProcessCommand();
            break;
        }

        default:
            SetStatus("Unknown key, ignoring input.");
            break;
        }
    }

    void KShell::ProcessCommand()
    {   
        //echo user input, then parse command
        inputLine[inputLength] = 0;
        WriteLine(inputLine);
        
        if (inputLength >= 5 && sl::memcmp(inputLine, "serial", 6) == 0)
        {
            echoSerialOut = !echoSerialOut;
            SetStatus("Serial echo toggled.");
        }
        if (inputLength > 2 && sl::memcmp(inputLine, "die", 3) == 0)
        {
            Panic("Manual trigger.");
        }
        if (inputLength > 5 && sl::memcmp(inputLine, "uptime", 6) == 0)
        {
            WriteLine(sl::UIntToString(SystemClock::The()->GetUptime(), 10).Data());
            SetStatus("Showing system uptime.");
        }
        
        //TODO: command parsing
        
        //clear input line and prompt
        sl::memset(inputLine, 0, inputLength);
        inputLength = inputCursor = 0;
        SetPrompt(nullptr);
    }

    void KShell::ClearLine(uint32_t lineNum, uint32_t startCol, uint32_t endCol, Colour clearColour)
    {
        if (endCol > (unsigned)displaySize.x)
            endCol = (unsigned)displaySize.x;
        uint32_t top = lineNum * (unsigned)characterSize.y;
        uint32_t left = startCol * (unsigned)characterSize.x;
        uint32_t height = (unsigned)characterSize.y;
        uint32_t width = (endCol - startCol) * (unsigned)characterSize.x;

        KRenderer::The()->DrawRect(Position(left, top), Position(width, height), clearColour, true);
    }

    void KShell::RedrawCursor()
    {
        Colour col = cursorVisible ? Colour(0xFFFFFFFF) : Colour(0x00000000);
        
        size_t bottom = KSHELL_PROMPT_LINE * characterSize.y + KSHELL_CURSOR_HEIGHT + KSHELL_CURSOR_SHIFT_Y;
        size_t top = bottom - KSHELL_CURSOR_HEIGHT;
        size_t left = dirtyLineLengths[KSHELL_PROMPT_LINE] * characterSize.x + KSHELL_CURSOR_SHIFT_X;
        size_t right = left + KSHELL_CURSOR_WIDTH;
        for (size_t x = left; x < right; x++)
        {
            for (size_t y = top; y < bottom; y++)
                KRenderer::The()->DrawPixel(Position(x, y), col);
        }
    }
    
    void KShell::Main()
    {
        Init();
        keepRunning = true;

        while (keepRunning)
        {
            //blink cursor (so satisfying)
            if (SystemClock::The()->GetUptime() >= nextCursorToggleMS)
            {
                nextCursorToggleMS += KSHELL_CURSOR_BLINK_MS;
                cursorVisible = !cursorVisible;
                RedrawCursor();
            }

            //clear status line if needed (and set it to never update until a new status is written)
            if (SystemClock::The()->GetUptime() >= statusClearTime)
            {
                ClearLine(KSHELL_STATUS_LINE, 8, dirtyLineLengths[KSHELL_STATUS_LINE], KSHELL_STATUS_BG_COLOUR);
                statusClearTime = UINT64_UPPER_LIMIT;
            }
            
            if (Ps2Keyboard::The()->KeysAvailable() || LogsUnread() > 0)
            {
                //check for unread logs, display them if we need to
                while (LogsUnread() > 0)
                {
                    size_t logsCount = KSHELL_INCOMING_LOGS_MAX;
                    ReceiveLogs(incomingLogsBuffer, &logsCount);
                    
                    for (size_t i = 0; i < logsCount; i++)
                        WriteLine(incomingLogsBuffer[i]);
                }
                
                //check for keyboard input
                unsigned int keysCount = 0;
                Ps2Keyboard::The()->GetKeys(incomingKeysBuffer, &keysCount);

                if (keysCount == 0)
                    continue; //jump into loop again, looks like someone else consumed keyboard keys

                for (size_t i = 0; i < keysCount; i++)
                    ProcessKey(incomingKeysBuffer[i]);
            }
            else
                Multiprocessing::Thread::Current()->Sleep(3); //sleep for a little bit
        }
    }
    
    void KShell::WriteLine(const char* const line)
    {
        if (writeCursorPos.y >= KSHELL_OUTPUT_LIMIT_LINE)
            writeCursorPos.y = 0; //roll over to the top if we ever reach the limit line

        if (dirtyLineLengths[writeCursorPos.y] > 0)
            ClearLine(writeCursorPos.y, 0, dirtyLineLengths[writeCursorPos.y], bgColour);
        
        KRenderer::The()->SetCursor(writeCursorPos);
        KRenderer::The()->Write(line);

        dirtyLineLengths[writeCursorPos.y] = KRenderer::The()->GetCursor().x;
        writeCursorPos.y++;

        if (echoSerialOut)
        {
            //more slow serial goodness :(
            int textLen = sl::memfirst(line, 0, 0);
            for (int i = 0; i < textLen; i++)
                SerialPort::COM1()->WriteByte(line[i]);
            SerialPort::COM1()->WriteByte('\n');
        }
    }

    void KShell::SetPrompt(const char* const text)
    {
        if (text != nullptr)
            promptText = text;
        if (dirtyLineLengths[KSHELL_PROMPT_LINE] > 0)
            ClearLine(KSHELL_PROMPT_LINE, 0, dirtyLineLengths[KSHELL_PROMPT_LINE] + 1, bgColour); //+1 because cursor might be visible, we force it to redraw so this'll prevent ghosting
        
        KRenderer::The()->SetCursor(Position(0, KSHELL_PROMPT_LINE));
        KRenderer::The()->Write(promptText);

        inputLine[inputLength] = 0;
        KRenderer::The()->Write(inputLine);

        dirtyLineLengths[KSHELL_PROMPT_LINE] = KRenderer::The()->GetCursor().x;
        KRenderer::The()->SetCursor(writeCursorPos);

        RedrawCursor();
    }

    void KShell::SetStatus(const char* const text)
    {
        if (dirtyLineLengths[KSHELL_STATUS_LINE] > 8)
            ClearLine(KSHELL_STATUS_LINE, 8, dirtyLineLengths[KSHELL_STATUS_LINE], KSHELL_STATUS_BG_COLOUR);
        
        if (text != nullptr)
        {
            KRenderer::The()->SetCursor(Position(8, KSHELL_STATUS_LINE));
            KRenderer::The()->Write(text);
        }
        
        dirtyLineLengths[KSHELL_STATUS_LINE] = KRenderer::The()->GetCursor().x;
        KRenderer::The()->SetCursor(writeCursorPos);

        statusClearTime = SystemClock::The()->GetUptime() + KSHELL_STATUS_HOLD_MS;
    }
}