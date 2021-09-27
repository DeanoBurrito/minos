#pragma once

#include <stdint.h>
#include <stddef.h>

#define PSF1_MAGIC_0 0x36
#define PSF1_MAGIC_1 0x04

namespace Kernel::Shell
{
    struct PSF1Header
    {
        uint8_t magic[2];
        uint8_t mode;
        uint8_t charSize;
    } __attribute__((packed));
    
    class PSF1Font
    {
    public:
        PSF1Header* header;
        void* glyphs;
        size_t width;
        size_t height;

        PSF1Font();
        PSF1Font(void* data, size_t w = 8, size_t h = 16);

        bool Valid();
    };

    //TODO: PSF v2 support https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
}
