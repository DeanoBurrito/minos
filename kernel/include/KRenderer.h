#pragma once

#include <stdint.h>
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

    struct Colour
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;

        Colour(uint32_t rgba)
        {
            red = (rgba & 0xFF000000) >> 24;
            green = (rgba & 0x00FF0000) >> 16;
            blue = (rgba & 0x0000FF00) >> 8;
            alpha = (rgba & 0x000000FF) >> 0;
        }

        Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            red = r;
            green = g;
            blue = b;
            alpha = a;
        }

        Colour(Colour rgb, uint8_t newAlpha)
        {
            red = rgb.red;
            green = rgb.green;
            blue = rgb.blue;
            alpha = newAlpha;
        }

        Colour() : Colour(0x00000000)
        {
        }

        constexpr uint32_t GetFormatted(FramebufferPixelFormat format) const
        {
            uint32_t colour = 0x00000000;
            switch (format)
            {
            case FramebufferPixelFormat::BlueGreenRedAlpha_32BPP:
                colour = colour | ((uint32_t)alpha << 24);
                colour = colour | ((uint32_t)red << 16);
                colour = colour | ((uint32_t)green << 8);
                colour = colour | ((uint32_t)blue << 0);
                return colour;
            case FramebufferPixelFormat::RedGreenBlueAlpha_32BPP:
                colour = colour | ((uint32_t)alpha << 24);
                colour = colour | ((uint32_t)blue << 16);
                colour = colour | ((uint32_t)green << 8);
                colour = colour | ((uint32_t)red << 0);
                return colour;

            default:
                return colour;
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
        Colour bgColour;
        Colour fgColour;

    public:
        static KRenderer* The();
        void Init(const BootInfo* bootInfo);

        void Clear(const Colour clearColour = Colour(0x000000ff));
        void DrawPixel(const Position where, const Colour col);
        void DrawLine(const Position start, const Position end, const Colour col);
        void DrawRect(const Position topLeft, const Position size, const Colour col, const bool filled);
        void DrawChar(const char c, const Position where, const Colour fg, const Colour bg);
        void DrawText(const char* text, const Position where, const Colour col);
        void DrawText(const char* text, const Position whre, const Colour fg, const Colour bg);

        void SetCursor(const Position where);
        Position GetCursor();
        void SetColours(const Colour foreground, const Colour background);
        void Write(const char* text);
        void WriteLine(const char* text);

        Position GetFramebufferSize();
        Position GetFontCharacterSize();
    };
}
