#include "KRenderer.h"

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
            break;
        }

        //TODO: emit log message if pixel format is invalid, or any other details are wonky.
        font = bootInfo->font;
        fontWidth = 8;
        fontHeight = 16;

        cursorPos = 0;
        bgColor = Color(0x000000FF);
        fgColor = Color(0xFFFFFFFF);

        Clear();
    }

    void KRenderer::Clear(const Color clearColor)
    {
        int bytesPerPixel = framebuffer.bitsPerPixel / 8;
        uint32_t formattedColor = clearColor.GetFormatted(framebuffer.pixelFormat);

        for (unsigned y = 0; y < framebuffer.height; y++)
        {
            for (unsigned int x = 0; x < framebuffer.width * bytesPerPixel; x += bytesPerPixel)
            {
                *(unsigned int *)(x + (y * framebuffer.pixelsPerScanline * bytesPerPixel) + framebuffer.baseAddress) = formattedColor;
            }
        }
    }

    void KRenderer::DrawPixel(const Position where, const Color col)
    {
        //oof, thats a long one.
        int bytesPerPixel = framebuffer.bitsPerPixel / 8;
        *(unsigned int *)(where.x * bytesPerPixel + (where.y * framebuffer.pixelsPerScanline * bytesPerPixel) + framebuffer.baseAddress) = col.GetFormatted(framebuffer.pixelFormat);
    }

    void KRenderer::DrawLine(const Position start, const Position end, const Color col)
    {
    }

    void KRenderer::DrawRect(const Position topLeft, const Position size, const Color col, const bool filled)
    {
    }

    void KRenderer::DrawChar(const char c, const Position where, const Color fgCol, const Color bgCol)
    {
        unsigned int *pixPtr = (unsigned int*)framebuffer.baseAddress;
        uint32_t fgFormatted = fgCol.GetFormatted(framebuffer.pixelFormat);
        uint32_t bgFormatted = bgCol.GetFormatted(framebuffer.pixelFormat);

        char* fontPtr = (char*)font->glyphBuffer + (c * font->psf1_haeder->charSize);

        for (unsigned long y = where.y; y < where.y + fontHeight; y++)
        {
            for (unsigned long x = where.x; x < where.x + fontWidth; x++)
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

    void KRenderer::DrawText(const char* text, const Position where, const Color col)
    {
        DrawText(text, where, col, Color(0x00000000));
    }

    void KRenderer::DrawText(const char* text, const Position where, const Color fg, const Color bg)
    {
        int len = 0;
        while (text[len] != 0)
            len++;

        Position lastPos = where;
        for (int i = 0; i < len; i++)
        {   
            DrawChar(text[i], lastPos, fg, bg);
            lastPos.x += fontWidth;
        }
    }

    void KRenderer::SetCursor(const int x, const int y)
    {
        if (x < 0 || y < 0)
            return;
        if (x > framebuffer.width / fontWidth || y > framebuffer.height / fontHeight)
            return;

        cursorPos.x = x;
        cursorPos.y = y;
    }

    void KRenderer::SetColors(const Color foreground, const Color background)
    {
        if (foreground.alpha > 0)
            fgColor = foreground;
        if (background.alpha > 0)
            bgColor = background;
    }

    void KRenderer::Write(const char* text)
    {
        int len = 0;
        while (text[len] != 0)
            len++;

        DrawText(text, Position(cursorPos.x * fontWidth, cursorPos.y * fontHeight), fgColor, bgColor);
        cursorPos.x += len;
    }

    void KRenderer::WriteLine(const char* text)
    {
        DrawText(text, Position(cursorPos.x * fontWidth, cursorPos.y * fontHeight), fgColor, bgColor);
        cursorPos.x = 0;
        cursorPos.y += 1;
    }
}
