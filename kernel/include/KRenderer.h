#pragma once

#include <stdint-gcc.h>
#include <BootInfo.h>

namespace Kernel
{

    enum class FramebufferPixelFormat
    {
        Unknown,
        RedGreenBlueAlpha_32BPP,
        BlueGreenRedAlpha_32BPP,
    };

    struct Position
    {
        int x;
        int y;

        Position(int nx, int ny)
        {
            x = nx;
            y = ny;
        }

        Position(int xy) : Position(xy, xy)
        {
        }

        Position() : Position(0)
        {
        }
    };

    struct Color
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;

        Color(uint32_t rgba)
        {
            red = (rgba & 0xFF000000) >> 24;
            green = (rgba & 0x00FF0000) >> 16;
            blue = (rgba & 0x0000FF00) >> 8;
            alpha = (rgba & 0x000000FF) >> 0;
        }

        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            red = r;
            green = g;
            blue = b;
            alpha = a;
        }

        Color(Color rgb, uint8_t newAlpha)
        {
            red = rgb.red;
            green = rgb.green;
            blue = rgb.blue;
            alpha = newAlpha;
        }

        Color() : Color(0x00000000)
        {
        }

        constexpr uint32_t GetFormatted(FramebufferPixelFormat format) const
        {
            uint32_t color = 0x00000000;
            switch (format)
            {
            case FramebufferPixelFormat::BlueGreenRedAlpha_32BPP:
                color = color | ((uint32_t)alpha << 24);
                color = color | ((uint32_t)red << 16);
                color = color | ((uint32_t)green << 8);
                color = color | ((uint32_t)blue << 0);
                return color;
            case FramebufferPixelFormat::RedGreenBlueAlpha_32BPP:
                color = color | ((uint32_t)alpha << 24);
                color = color | ((uint32_t)blue << 16);
                color = color | ((uint32_t)green << 8);
                color = color | ((uint32_t)red << 0);
                return color;

            default:
                return color;
            }
        }
    };

    struct GopFramebuffer
    {
        uint64_t baseAddress;
        size_t bufferSize;
        uint32_t width;
        uint32_t height;
        uint32_t pixelsPerScanline;
        FramebufferPixelFormat pixelFormat;
        uint8_t bitsPerPixel;
    };

    class KRenderer
    {
    private:
        GopFramebuffer framebuffer;
        PSF1_Font *font;
        unsigned int fontWidth;
        unsigned int fontHeight;

        Position cursorPos;
        Color bgColor;
        Color fgColor;

    public:
        static KRenderer* The();
        void Init(const BootInfo* bootInfo);

        void Clear(const Color clearColor = Color(0x000000ff));
        void DrawPixel(const Position where, const Color col);
        void DrawLine(const Position start, const Position end, const Color col);
        void DrawRect(const Position topLeft, const Position size, const Color col, const bool filled);
        void DrawChar(const char c, const Position where, const Color fg, const Color bg);
        void DrawText(const char* text, const Position where, const Color col);
        void DrawText(const char* text, const Position whre, const Color fg, const Color bg);

        void SetCursor(const int x, const int y);
        void SetColors(const Color foreground, const Color background);
        void Write(const char* text);
        void WriteLine(const char* text);
    };
}
