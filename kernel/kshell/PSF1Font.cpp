#include <kshell/PSF1Font.h>
#include <Memory.h>

namespace Kernel::Shell
{
    PSF1Font::PSF1Font()
    {
        glyphs = header = nullptr;
        width = height = 0;
    }
    
    PSF1Font::PSF1Font(void* data, size_t w, size_t h)
    {
        header = reinterpret_cast<PSF1Header*>(data);
        sl::UIntPtr ptr(data);
        ptr.raw += sizeof(PSF1Header);
        glyphs = ptr.ptr;

        width = w;
        height = h;
    }
    
    bool PSF1Font::Valid()
    {
        return header != nullptr && header->magic[0] == PSF1_MAGIC_0 && header->magic[1] == PSF1_MAGIC_1;
    }
}