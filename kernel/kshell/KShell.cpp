#include <kshell/KShell.h>
#include <multiprocessing/Scheduler.h>
#include <StringUtil.h>
#include <memory/Utilities.h>
#include <Serial.h>
#include <KLog.h>

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
        memset(inputLine, 0, KSHELL_INPUT_BUFFER_LEN + 1);
        promptText = "(kernel) - no FS $ ";
        echoSerialOut = true;

        characterSize = KRenderer::The()->GetFontCharacterSize();
        displaySize = KRenderer::The()->GetFramebufferSize();
        displaySize = Position(displaySize.x / characterSize.x, displaySize.y / characterSize.y);
        KRenderer::The()->SetColours(fgColour, bgColour);
        KRenderer::The()->Clear();

        dirtyLineLengths = new uint8_t[displaySize.y];
        memset(dirtyLineLengths, 0, displaySize.y);

        SetPrompt(nullptr);
        KRenderer::The()->SetCursor(Position(0, KSHELL_STATUS_LINE));
        KRenderer::The()->Write("STATUS: ");
        SetStatus(nullptr);
        
        //TODO: disable log printing automatically, instead piping its output to us
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
                memcopy(inputLine + inputCursor, inputLine + inputCursor - 1, inputLength - inputCursor);
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
        //TODO: command parsing
        inputLine[inputLength] = 0;
        WriteLine(inputLine);
        memset(inputLine, 0, inputLength);
        inputLength = inputCursor = 0;
        SetPrompt(nullptr);
    }

    void KShell::ClearLine(uint32_t lineNum, uint32_t startCol, uint32_t endCol)
    {
        if (endCol > displaySize.x)
            endCol = displaySize.x;
        uint32_t top = lineNum * characterSize.y;
        uint32_t left = startCol * characterSize.x;
        uint32_t height = characterSize.y;
        uint32_t width = (endCol - startCol) * characterSize.x;

        KRenderer::The()->DrawRect(Position(left, top), Position(width, height), bgColour, true);
    }
    
    void KShell::Main()
    {
        Init();
        keepRunning = true;

        while (keepRunning)
        {
            if (Ps2Keyboard::The()->KeysAvailable() /* || Log::OutputPending? */)
            {
                //TODO: check for log input to display
                
                //check for keyboard input
                unsigned int keysCount = 0;
                Ps2Keyboard::The()->GetKeys(incomingKeysBuffer, &keysCount);

                if (keysCount == 0)
                    continue; //jump into loop again, looks like someone else consumed keyboard keys

                for (int i = 0; i < keysCount; i++)
                    ProcessKey(incomingKeysBuffer[i]);
            }
            else
            {
                //no point in doing anything else, user has no requested input
                Multiprocessing::Scheduler::The()->Yield();
            }
        }
    }
    
    void KShell::WriteLine(const char* const line)
    {
        if (writeCursorPos.y >= KSHELL_OUTPUT_LIMIT_LINE)
            writeCursorPos.y = 0; //roll over to the top if we ever reach the limit line

        if (dirtyLineLengths[writeCursorPos.y] > 0)
            ClearLine(writeCursorPos.y, 0, dirtyLineLengths[writeCursorPos.y]);
        
        KRenderer::The()->SetCursor(writeCursorPos);
        KRenderer::The()->Write(line);

        dirtyLineLengths[writeCursorPos.y] = KRenderer::The()->GetCursor().x;
        writeCursorPos.y++;

        if (echoSerialOut)
        {
            //more slow serial goodness :(
            int textLen = strlen(line);
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
            ClearLine(KSHELL_PROMPT_LINE, 0, dirtyLineLengths[KSHELL_PROMPT_LINE]);
        
        KRenderer::The()->SetCursor(Position(0, KSHELL_PROMPT_LINE));
        KRenderer::The()->Write(promptText);

        inputLine[inputLength] = 0;
        KRenderer::The()->Write(inputLine);

        dirtyLineLengths[KSHELL_PROMPT_LINE] = KRenderer::The()->GetCursor().x;
        KRenderer::The()->SetCursor(writeCursorPos);
    }

    void KShell::SetStatus(const char* const text)
    {
        if (dirtyLineLengths[KSHELL_STATUS_LINE] > 8)
            ClearLine(KSHELL_STATUS_LINE, 8, dirtyLineLengths[KSHELL_STATUS_LINE]);
        
        if (text != nullptr)
        {
            KRenderer::The()->SetCursor(Position(8, KSHELL_STATUS_LINE));
            KRenderer::The()->Write(text);
        }
        
        dirtyLineLengths[KSHELL_STATUS_LINE] = KRenderer::The()->GetCursor().x;
        KRenderer::The()->SetCursor(writeCursorPos);
    }
}