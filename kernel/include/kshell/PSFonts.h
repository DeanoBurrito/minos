#pragma once

#include <stdint.h>
#include <stddef.h>

#define PSF1_MAGIC_0 0x36
#define PSF1_MAGIC_1 0x04

#define PSF1_MODE_512 0x1
#define PSF1_MODE_HASTAB 0x2
#define PSF1_MODE_MODEHASSEQ 0x1

namespace Kernel::Shell
{
    struct PSF1Header
    {
        uint8_t magic[2];
        uint8_t mode;
        uint8_t charSize; //width is always 8, this is height.
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

#define PSF2_MAGIC_0 0x72
#define PSF2_MAGIC_1 0xB5
#define PSF2_MAGIC_2 0x4A
#define PSF2_MAGIC_3 0x86

#define PSF2_FLAG_HAS_UNICODE 0x1
//0ver baby
#define PSF2_MAX_VERSION 0
#define PSF2_SEPARATOR 0xFF
#define PSF2_STARTSEQ 0xFE

    struct PSF2Header
    {
        uint8_t magic[4];
        uint32_t version;
        uint32_t headerLength;
        uint32_t flags;
        uint32_t glyphsCount;
        uint32_t glyphByteWidth;
        uint32_t height;
        uint32_t width;
    };

    //TODO: implement PSF2 struct. Implement RenderChar(buffer, x, y, char) for both v1/v2, move basic font rendering to these fonts, since they know how to handle it best.
}
