#include <KRenderer.h>
#include <KLog.h>
#include <InitDisk.h>

namespace Kernel
{
    KRenderer globalRenderer;
    KRenderer* KRenderer::The()
    {
        return &globalRenderer;
    }

    void KRenderer::Init(const BootInfo* bootInfo)
    {
        framebuffer.baseAddress = (uint64_t)bootInfo->gop.baseAddress;
        framebuffer.bufferSize = bootInfo->gop.bufferSize;
        framebuffer.width = bootInfo->gop.width;
        framebuffer.height = bootInfo->gop.height;
        framebuffer.pixelsPerScanline = bootInfo->gop.pixelsPerScanline;

        switch (bootInfo->gop.pixelFormat)
        {
        case PIXEL_FORMAT_RedGreenBlueReserved_8BPP:
            framebuffer.pixelFormat = FramebufferPixelFormat::RedGreenBlueAlpha_32BPP;
            framebuffer.bitsPerPixel = 32;
            break;
        case PIXEL_FORMAT_BlueGreenRedReserved_8BPP:
            framebuffer.pixelFormat = FramebufferPixelFormat::BlueGreenRedAlpha_32BPP;
            framebuffer.bitsPerPixel = 32;
            break;
        default:
            framebuffer.pixelFormat = FramebufferPixelFormat::Unknown;
            framebuffer.bitsPerPixel = 1; //if its zero we'll get infinite loops when drawing multiple pixels.
            LogError("KRenderer found unknown framebuffer pixel format. Corrupt bootloader?");
            break;
        }

        size_t fontSize;
        void *fontStart, *fontEnd;
        if (!GetFileData("zap-light16.psf", &fontStart, &fontEnd, &fontSize))
        {
            LogError("Could not load font file from initdisk. Terminal issue (hahaha).");
            return;
        }
        font = Shell::PSF1Font(fontStart);

        cursorPos = 0;
        bgColour = Colour(0x000000FF);
        fgColour = Colour(0xFFFFFFFF);

        Clear();
    }

    void KRenderer::Clear(const Colour clearColour)
    {
        int bytesPerPixel = framebuffer.bitsPerPixel / 8;
        uint32_t formattedColour = clearColour.GetFormatted(framebuffer.pixelFormat);

        for (unsigned y = 0; y < framebuffer.height; y++)
        {
            for (unsigned int x = 0; x < framebuffer.width * bytesPerPixel; x += bytesPerPixel)
            {
                *(unsigned int *)(x + (y * framebuffer.pixelsPerScanline * bytesPerPixel) + framebuffer.baseAddress) = formattedColour;
            }
        }
    }

    void KRenderer::DrawPixel(const Position where, const Colour col)
    {
        //oof, thats a long one.
        int bytesPerPixel = framebuffer.bitsPerPixel / 8;
        *(unsigned int *)(where.x * bytesPerPixel + (where.y * framebuffer.pixelsPerScanline * bytesPerPixel) + framebuffer.baseAddress) = col.GetFormatted(framebuffer.pixelFormat);
    }

    void KRenderer::DrawLine(const Position start, const Position end, const Colour col)
    {
    }

    void KRenderer::DrawRect(const Position topLeft, const Position size, const Colour col, const bool filled)
    {
        if (filled)
        {
            for (int x = topLeft.x; x < topLeft.x + size.x; x++)
            {
                for (int y = topLeft.y; y < topLeft.y + size.y; y++)
                {
                    DrawPixel(Position(x, y), col);
                }
            }
        }
        else
        {
            //TODO: draw 4 lines
        }
    }

    void KRenderer::DrawChar(const char c, const Position where, const Colour fgCol, const Colour bgCol)
    {
        unsigned int *pixPtr = (unsigned int*)framebuffer.baseAddress;
        uint32_t fgFormatted = fgCol.GetFormatted(framebuffer.pixelFormat);
        //uint32_t bgFormatted = bgCol.GetFormatted(framebuffer.pixelFormat); //NOTE: unused, cant current draw backgrounds with how we draw text right now.

        char* fontPtr = (char*)font.glyphs + (c * font.header->charSize);

        for (unsigned long y = where.y; y < where.y + font.height; y++)
        {
            for (unsigned long x = where.x; x < where.x + font.width; x++)
            {
                if ((*fontPtr & (0b10000000 >> (x - where.x))) > 0)
                {
                    //pixel is lit
                    *(unsigned int*)(pixPtr + x + (y * framebuffer.pixelsPerScanline)) = fgFormatted;
                }
            }
            fontPtr++;
        }
    }

    void KRenderer::DrawText(const char* text, const Position where, const Colour col)
    {
        DrawText(text, where, col, Colour(0x00000000));
    }

    void KRenderer::DrawText(const char* text, const Position where, const Colour fg, const Colour bg)
    {
        int len = 0;
        while (text[len] != 0)
            len++;

        Position lastPos = where;
        for (int i = 0; i < len; i++)
        {   
            DrawChar(text[i], lastPos, fg, bg);
            lastPos.x += font.width;
        }
    }

    void KRenderer::SetCursor(const Position where)
    {
        if (where.x < 0 || where.y < 0)
            return;
        if ((unsigned)where.x > framebuffer.width / font.width || (unsigned)where.y > framebuffer.height / font.height)
            return;

        cursorPos.x = where.x;
        cursorPos.y = where.y;
    }

    Position KRenderer::GetCursor()
    {
        return cursorPos;
    }

    void KRenderer::SetColours(const Colour foreground, const Colour background)
    {
        if (foreground.alpha > 0)
            fgColour = foreground;
        if (background.alpha > 0)
            bgColour = background;
    }

    void KRenderer::Write(const char* text)
    {
        int len = 0;
        while (text[len] != 0)
            len++;

        DrawText(text, Position(cursorPos.x * font.width, cursorPos.y * font.height), fgColour, bgColour);
        cursorPos.x += len;
    }

    void KRenderer::WriteLine(const char* text)
    {
        DrawText(text, Position(cursorPos.x * font.width, cursorPos.y * font.height), fgColour, bgColour);
        cursorPos.x = 0;
        cursorPos.y += 1;
    }

    Position KRenderer::GetFramebufferSize()
    {
        return Position(framebuffer.width, framebuffer.height);
    }

    Position KRenderer::GetFontCharacterSize()
    {
        return Position(font.width, font.height);
    }
}
